//===-- HLVM to LLVM Code Generator -----------------------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/CodeGen/LLVMGenerator.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the implementation of the HLVM -> LLVM Code Generator
//===----------------------------------------------------------------------===//

#include <hlvm/CodeGen/LLVMGenerator.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/Base/Assert.h>
#include <hlvm/Pass/Pass.h>
#include <llvm/Module.h>
#include <llvm/PassManager.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <llvm/DerivedTypes.h>
#include <llvm/TypeSymbolTable.h>
#include <llvm/Constants.h>
#include <llvm/CallingConv.h>
#include <llvm/Linker.h>
#include <llvm/Analysis/LoadValueNumbering.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/Parser.h>
#include <llvm/Bytecode/Writer.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Analysis/Dominators.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/CommandLine.h>

namespace llvm {
  void dump(llvm::Value* V) {
    V->dump();
  }
  void dumpType(llvm::Type* T) {
    T->dump();
  }
}

namespace 
{

using namespace hlvm;

class LLVMGeneratorPass : public hlvm::Pass
{
  typedef std::vector<llvm::Module*> ModuleList;
  typedef std::vector<llvm::BranchInst*> BranchList;
  typedef std::vector<llvm::BasicBlock*> BlockStack;
  typedef std::map<const hlvm::Operator*,llvm::Value*> OperandMap;
  typedef std::map<const hlvm::Block*,llvm::BasicBlock*> BlockMap;
  typedef std::map<const hlvm::Operator*,llvm::BasicBlock*> LoopMap;
  typedef std::map<const hlvm::Block*,llvm::Instruction*> ResultsMap;
  typedef std::map<const hlvm::Variable*,llvm::Value*> VariableMap;
  typedef std::map<const hlvm::AutoVarOp*,llvm::Value*> AutoVarMap;
  typedef std::map<const hlvm::ConstantValue*,llvm::Constant*> ConstantMap;
  typedef std::map<const hlvm::Function*,llvm::Function*> FunctionMap;
  ModuleList modules;           ///< The list of modules we construct
  llvm::Module*     lmod;       ///< The current module we're generation 
  llvm::Function*   lfunc;      ///< The current LLVM function we're generating 
  llvm::BasicBlock* lblk;       ///< The current LLVM block we're generating
  OperandMap operands;          ///< The current list of instruction operands
  BlockMap enters;              ///< Map of Block to entry BasicBlock
  BlockMap exits;               ///< Map of Block to exit BasicBlock
  BlockStack blocks;            ///< The stack of blocks we're constructing
  BranchList breaks;            ///< The list of break instructions to fix up
  BranchList continues;         ///< The list of continue instructions to fix up
  VariableMap gvars;            ///< Map of HLVM -> LLVM gvars
  AutoVarMap lvars;             ///< Map of HLVM -> LLVM auto vars
  llvm::TypeSymbolTable ltypes; ///< The cached LLVM types we've generated
  ConstantMap consts;           ///< The cached LLVM constants we've generated
  FunctionMap funcs;            ///< The cached LLVM constants we've generated
  const AST* ast;               ///< The current Tree we're traversing
  const Bundle* bundle;         ///< The current Bundle we're traversing
  const hlvm::Function* function;  ///< The current Function we're traversing
  const Block* block;           ///< The current Block we're traversing
  std::vector<llvm::Function*> progs; ///< The list of programs to emit

  /// Interfaces to the HLVM Runtime Library
  llvm::PointerType*  hlvm_text;          ///< Opaque type for text objects
  llvm::Function*     hlvm_text_create;   ///< Create a new text object
  llvm::Function*     hlvm_text_delete;   ///< Delete a text object
  llvm::Function*     hlvm_text_to_buffer;///< Convert text to a buffer
  llvm::PointerType*  hlvm_buffer;        ///< Pointer To octet
  llvm::Function*     hlvm_buffer_create; ///< Create a new buffer object
  llvm::Function*     hlvm_buffer_delete; ///< Delete a buffer
  llvm::PointerType*  hlvm_stream;        ///< Pointer to stream type
  llvm::Function*     hlvm_stream_open;   ///< Function for stream_open
  llvm::Function*     hlvm_stream_read;   ///< Function for stream_read
  llvm::Function*     hlvm_stream_write_buffer; ///< Write buffer to stream
  llvm::Function*     hlvm_stream_write_text;   ///< Write text to stream
  llvm::Function*     hlvm_stream_write_string; ///< Write string to stream
  llvm::Function*     hlvm_stream_close;  ///< Function for stream_close
  llvm::FunctionType* hlvm_program_signature; ///< The llvm type for programs

  public:
    LLVMGeneratorPass(const AST* tree)
      : Pass(0,Pass::PreAndPostOrderTraversal),
      modules(), lmod(0), lfunc(0), lblk(0), operands(), blocks(), breaks(), 
      continues(),
      gvars(), lvars(), ltypes(), consts(), funcs(),
      ast(tree),   bundle(0), function(0), block(0),
      hlvm_text(0), hlvm_text_create(0), hlvm_text_delete(0),
      hlvm_text_to_buffer(0),
      hlvm_buffer(0), hlvm_buffer_create(0), hlvm_buffer_delete(0),
      hlvm_stream(0),
      hlvm_stream_open(0), hlvm_stream_read(0),
      hlvm_stream_write_buffer(0), hlvm_stream_write_text(0), 
      hlvm_stream_write_string(0),
      hlvm_stream_close(0), hlvm_program_signature(0)
      { }
    ~LLVMGeneratorPass() { }

  /// Conversion functions
  const llvm::Type* getType(const hlvm::Type* ty);
  llvm::Constant* getConstant(const hlvm::ConstantValue* C);
  llvm::Value* getVariable(const hlvm::Variable* V);
  llvm::Function* getFunction(const hlvm::Function* F);
  inline llvm::GlobalValue::LinkageTypes getLinkageTypes(LinkageKinds lk);
  inline std::string getLinkageName(const Linkable* li);
  inline llvm::Value* getBoolean(llvm::Value* op);
  inline llvm::Value* getInteger(llvm::Value* op);
  inline llvm::Value* getReferent(hlvm::ReferenceOp* r);
  inline llvm::Value* toBoolean(llvm::Value* op);
  inline llvm::Value* ptr2Value(llvm::Value* op);
  inline llvm::Value* coerce(llvm::Value* op);
  inline void pushOperand(llvm::Value* v, const Operator* op);
  inline llvm::Value* popOperand(const Operator*op);
  inline llvm::Value* popOperandAsBlock(
    const Operator* op, const std::string&,
    llvm::BasicBlock*& entry_block, llvm::BasicBlock*& exit_block);
  inline llvm::Value* popOperandAsCondition(
    const Operator* op, const std::string&,
    llvm::BasicBlock*& entry_block, llvm::BasicBlock*& exit_block);
  inline llvm::BasicBlock* newBlock(const std::string& name);
  inline llvm::BasicBlock* pushBlock(const std::string& name);
  inline llvm::BasicBlock* popBlock(llvm::BasicBlock* curBlock);
  inline bool hasResult(hlvm::Block* B) const;
  llvm::AllocaInst* getOperatorResult(Operator* op, const std::string& name);
  llvm::Value* getBlockResult(Block* blk);
  inline void branchIfNotTerminated(
    llvm::BasicBlock* to, llvm::BasicBlock* from);

  inline void startNewFunction(llvm::Function* f);
  void resolveBranches(BranchList& list, llvm::BasicBlock* exit);

  /// Accessors for HLVM Runtime Library things
  inline llvm::Type*         get_hlvm_size();
  inline llvm::PointerType*  get_hlvm_text();
  inline llvm::Function*     get_hlvm_text_create();
  inline llvm::Function*     get_hlvm_text_delete();
  inline llvm::Function*     get_hlvm_text_to_buffer();
  inline llvm::PointerType*  get_hlvm_buffer();
  inline llvm::Function*     get_hlvm_buffer_create();
  inline llvm::Function*     get_hlvm_buffer_delete();
  inline llvm::PointerType*  get_hlvm_stream();
  inline llvm::Function*     get_hlvm_stream_open();
  inline llvm::Function*     get_hlvm_stream_read();
  inline llvm::Function*     get_hlvm_stream_write_buffer();
  inline llvm::Function*     get_hlvm_stream_write_text();
  inline llvm::Function*     get_hlvm_stream_write_string();
  inline llvm::Function*     get_hlvm_stream_close();
  inline llvm::FunctionType* get_hlvm_program_signature();

  /// Generator
  template <class NodeClass>
  inline void gen(NodeClass *nc);

  void genProgramLinkage();

  virtual void handleInitialize(AST* tree);
  virtual void handle(Node* n,Pass::TraversalKinds mode);
  virtual void handleTerminate();

  inline llvm::Module* linkModules();
};

llvm::Type*
LLVMGeneratorPass::get_hlvm_size()
{
  return llvm::Type::ULongTy;
}

llvm::PointerType*
LLVMGeneratorPass::get_hlvm_text()
{
  if (! hlvm_text) {
    llvm::OpaqueType* opq = llvm::OpaqueType::get();
    lmod->addTypeName("hlvm_text_obj", opq);
    hlvm_text = llvm::PointerType::get(opq);
    lmod->addTypeName("hlvm_text", hlvm_text);
  }
  return hlvm_text;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_text_create()
{
  if (! hlvm_text_create) {
    llvm::Type* result = get_hlvm_text();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_text_create",FT);
    hlvm_text_create = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_text_create", lmod);
  }
  return hlvm_text_create;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_text_delete()
{
  if (! hlvm_text_delete) {
    llvm::Type* result = get_hlvm_text();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_text());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_text_delete",FT);
    hlvm_text_delete = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_text_delete", lmod);
  }
  return hlvm_text_delete;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_text_to_buffer()
{
  if (! hlvm_text_to_buffer) {
    llvm::Type* result = get_hlvm_buffer();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_text());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_text_to_buffer_signature",FT);
    hlvm_text_to_buffer = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_text_to_buffer", lmod);
  }
  return hlvm_text_to_buffer;
}

llvm::PointerType*
LLVMGeneratorPass::get_hlvm_buffer()
{
  if (! hlvm_buffer) {
    llvm::OpaqueType* opq = llvm::OpaqueType::get();
    lmod->addTypeName("hlvm_buffer_obj", opq);
    hlvm_buffer = llvm::PointerType::get(opq);
    lmod->addTypeName("hlvm_buffer", hlvm_buffer);
  }
  return hlvm_buffer;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_buffer_create()
{
  if (! hlvm_buffer_create) {
    llvm::Type* result = get_hlvm_buffer();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_size());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_buffer_create",FT);
    hlvm_buffer_create = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_buffer_create", lmod);
  }
  return hlvm_buffer_create;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_buffer_delete()
{
  if (! hlvm_buffer_delete) {
    llvm::Type* result = get_hlvm_buffer();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_buffer());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_buffer_delete",FT);
    hlvm_buffer_delete = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
      "hlvm_buffer_delete", lmod);
  }
  return hlvm_buffer_delete;
}

llvm::PointerType* 
LLVMGeneratorPass::get_hlvm_stream()
{
  if (! hlvm_stream) {
    llvm::OpaqueType* opq = llvm::OpaqueType::get();
    lmod->addTypeName("hlvm_stream_obj", opq);
    hlvm_stream= llvm::PointerType::get(opq);
    lmod->addTypeName("hlvm_stream", hlvm_stream);
  }
  return hlvm_stream;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_stream_open()
{
  if (!hlvm_stream_open) {
    llvm::Type* result = get_hlvm_stream();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_stream_open_signature",FT);
    hlvm_stream_open = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage, 
        "hlvm_stream_open", lmod);
  }
  return hlvm_stream_open;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_stream_read()
{
  if (!hlvm_stream_read) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(get_hlvm_buffer());
    arg_types.push_back(get_hlvm_size());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_stream_read_signature",FT);
    hlvm_stream_read = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_read", lmod);
  }
  return hlvm_stream_read;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_stream_write_buffer()
{
  if (!hlvm_stream_write_buffer) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(get_hlvm_buffer());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_stream_write_buffer_signature",FT);
    hlvm_stream_write_buffer = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_write_buffer", lmod);
  }
  return hlvm_stream_write_buffer;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_stream_write_string()
{
  if (!hlvm_stream_write_string) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_stream_write_string_signature",FT);
    hlvm_stream_write_string = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_write_string", lmod);
  }
  return hlvm_stream_write_string;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_stream_write_text()
{
  if (!hlvm_stream_write_text) {
    llvm::Type* result = get_hlvm_size();
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    arg_types.push_back(get_hlvm_text());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_stream_write_text_signature",FT);
    hlvm_stream_write_text = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_write_text", lmod);
  }
  return hlvm_stream_write_text;
}

llvm::Function*
LLVMGeneratorPass::get_hlvm_stream_close()
{
  if (!hlvm_stream_close) {
    llvm::Type* result = llvm::Type::VoidTy;
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(get_hlvm_stream());
    llvm::FunctionType* FT = llvm::FunctionType::get(result,arg_types,false);
    lmod->addTypeName("hlvm_stream_close_signature",FT);
    hlvm_stream_close = 
      new llvm::Function(FT, llvm::GlobalValue::ExternalLinkage,
      "hlvm_stream_close", lmod);
  }
  return hlvm_stream_close;
}

llvm::FunctionType*
LLVMGeneratorPass::get_hlvm_program_signature()
{
  if (!hlvm_program_signature) {
    // Get the type of function that all entry points must have
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::Type::IntTy);
    arg_types.push_back(
      llvm::PointerType::get(llvm::PointerType::get(llvm::Type::SByteTy)));
    hlvm_program_signature = 
      llvm::FunctionType::get(llvm::Type::IntTy,arg_types,false);
    lmod->addTypeName("hlvm_program_signature",hlvm_program_signature);
  }
  return hlvm_program_signature;
}

std::string
LLVMGeneratorPass::getLinkageName(const Linkable* lk)
{
  // if (lk->isProgram())
    // return std::string("_hlvm_entry_") + lk->getName();
  // FIXME: This needs to incorporate the bundle name
  return lk->getName();
}

const llvm::Type*
LLVMGeneratorPass::getType(const hlvm::Type* ty)
{
  // First, lets see if its cached already
  const llvm::Type* result = ltypes.lookup(ty->getName());
  if (result)
    return result;

  // Okay, we haven't seen this type before so let's construct it
  switch (ty->getID()) {
    case BooleanTypeID:            result = llvm::Type::BoolTy; break;
    case CharacterTypeID:          result = llvm::Type::UShortTy; break;
    case OctetTypeID:              result = llvm::Type::SByteTy; break;
    case UInt8TypeID:              result = llvm::Type::UByteTy; break;
    case UInt16TypeID:             result = llvm::Type::UShortTy; break;
    case UInt32TypeID:             result = llvm::Type::UIntTy; break;
    case UInt64TypeID:             result = llvm::Type::ULongTy; break;
    case UInt128TypeID: 
      hlvmNotImplemented("128 bit primitive integer");
      break;
    case SInt8TypeID:              result = llvm::Type::SByteTy; break;
    case SInt16TypeID:             result = llvm::Type::ShortTy; break;
    case SInt32TypeID:             result = llvm::Type::IntTy; break;
    case SInt64TypeID:             result = llvm::Type::LongTy; break;
    case SInt128TypeID: 
      hlvmNotImplemented("128 bit primitive integer");
      break;
    case Float32TypeID:             result = llvm::Type::FloatTy; break;
    case Float64TypeID:             result = llvm::Type::DoubleTy; break;
    case Float44TypeID: 
    case Float80TypeID: 
    case Float128TypeID: 
      hlvmNotImplemented("extended and quad floating point");
      break;
    case AnyTypeID:
      hlvmNotImplemented("Any Type");
      break;
    case StringTypeID:              
      result = llvm::PointerType::get(llvm::Type::SByteTy);
      break;
    case IntegerTypeID:
      hlvmNotImplemented("arbitrary precision integer");
      break;
    case RealTypeID:
      hlvmNotImplemented("arbitrary precision real");
    case TextTypeID:
      result = get_hlvm_text();
      break;
    case StreamTypeID: 
      result = get_hlvm_stream();
      break;
    case BufferTypeID: 
      result = get_hlvm_buffer();
      break;
    case PointerTypeID: 
    {
      const hlvm::Type* hElemType = 
        llvm::cast<hlvm::PointerType>(ty)->getElementType();
      const llvm::Type* lElemType = getType(hElemType);
      result = llvm::PointerType::get(lElemType);

      // If the element type is opaque then we need to add a type name for this
      // pointer type because all opaques are unique unless named similarly.
      if (llvm::isa<llvm::OpaqueType>(lElemType))
        lmod->addTypeName(ty->getName(), result);
      break;
    }
    case VectorTypeID: {
      const hlvm::VectorType* VT = llvm::cast<hlvm::VectorType>(ty);
      const llvm::Type* elemType = getType(VT->getElementType());
      result = llvm::ArrayType::get(elemType, VT->getSize());
      break;
    }
    case ArrayTypeID: {
      const hlvm::ArrayType* AT = llvm::cast<hlvm::ArrayType>(ty);
      const llvm::Type* elemType = getType(AT->getElementType());
      std::vector<const llvm::Type*> Fields;
      Fields.push_back(llvm::Type::UIntTy);
      Fields.push_back(llvm::PointerType::get(elemType));
      result = llvm::StructType::get(Fields);
      break;
    }
    case StructureTypeID: {
      const hlvm::StructureType* ST = llvm::cast<hlvm::StructureType>(ty);
      std::vector<const llvm::Type*> Fields;
      for (StructureType::const_iterator I = ST->begin(), E = ST->end(); 
           I != E; ++I)
        Fields.push_back(getType((*I)->getType()));
      result = llvm::StructType::get(Fields);
      break;
    }
    case SignatureTypeID:
    {
      std::vector<const llvm::Type*> params;
      const SignatureType* st = llvm::cast<SignatureType>(ty);
      for (SignatureType::const_iterator I = st->begin(), E = st->end(); 
           I != E; ++I)
        params.push_back(getType((*I)->getType()));
      result = llvm::FunctionType::get(
        getType(st->getResultType()),params,st->isVarArgs());
      break;
    }
    case OpaqueTypeID: {
      return llvm::OpaqueType::get();
      break;
    }
    default:
      hlvmDeadCode("Invalid type code");
      break;
  }
  if (result)
    ltypes.insert(ty->getName(),result);
  return result;
}

llvm::Constant*
LLVMGeneratorPass::getConstant(const hlvm::ConstantValue* C)
{
  hlvmAssert(C!=0);
  hlvmAssert(C->isConstantValue());

  // First, lets see if its cached already
  ConstantMap::iterator I = 
    consts.find(const_cast<hlvm::ConstantValue*>(C));
  if (I != consts.end())
    return I->second;

  const hlvm::Type* hType = C->getType();
  const llvm::Type* lType = getType(hType);
  llvm::Constant* result = 0;
  switch (C->getID()) 
  {
    case ConstantBooleanID:
    {
      const ConstantBoolean* CI = llvm::cast<const ConstantBoolean>(C);
      result = llvm::ConstantBool::get(CI->getValue());
      break;
    }
    case ConstantIntegerID:
    {
      const ConstantInteger* CI = llvm::cast<const ConstantInteger>(C);
      const IntegerType* iType = llvm::cast<IntegerType>(hType);
      if (iType->isSigned()) {
        int64_t val = strtoll(CI->getValue().c_str(),0,CI->getBase());
        result = llvm::ConstantSInt::get(lType,val);
      }
      else {
        uint64_t val = strtoull(CI->getValue().c_str(),0,CI->getBase());
        result = llvm::ConstantUInt::get(lType,val);
      }
      break;
    }
    case ConstantRealID:
    {
      const ConstantReal* CR = llvm::cast<const ConstantReal>(C);
      double  val = strtod(CR->getValue().c_str(),0);
      result = llvm::ConstantFP::get(lType, val);
      break;
    }
    case ConstantStringID:
    {
      const ConstantString* CT = llvm::cast<ConstantString>(C);
      llvm::Constant* CA = llvm::ConstantArray::get(CT->getValue(), true);
      result = new llvm::GlobalVariable(CA->getType(), true, 
          llvm::GlobalValue::InternalLinkage, CA, C->getName(), lmod);
      break;
    }
    default:
      break;
  }
  if (result)
    consts[const_cast<hlvm::ConstantValue*>(C)] = result;
  else
    hlvmDeadCode("Didn't find constant");
  return result;
}

llvm::Value*
LLVMGeneratorPass::getVariable(const Variable* V) 
{
  hlvmAssert(V != 0);
  hlvmAssert(V->is(VariableID));

  // First, lets see if its cached already
  VariableMap::iterator I = 
    gvars.find(const_cast<hlvm::Variable*>(V));
  if (I != gvars.end())
    return I->second;

  // Not found, create it
  llvm::Constant* Initializer = 0;
  if (V->hasInitializer())
    Initializer = getConstant(V->getInitializer());
  else
    Initializer = llvm::Constant::getNullValue(getType(V->getType()));
  llvm::Value* gv = new llvm::GlobalVariable(
    /*Ty=*/ getType(V->getType()),
    /*isConstant=*/ false,
    /*Linkage=*/ getLinkageTypes(V->getLinkageKind()), 
    /*Initializer=*/ Initializer,
    /*Name=*/ getLinkageName(V),
    /*Parent=*/ lmod
  );
  gvars[V] = gv;
  return gv;
}

llvm::Function*
LLVMGeneratorPass::getFunction(const hlvm::Function* F)
{
  hlvmAssert(F != 0);
  hlvmAssert(F->is(FunctionID));

  // First, lets see if its cached already
  FunctionMap::iterator I = funcs.find(const_cast<hlvm::Function*>(F));
  if (I != funcs.end())
    return I->second;

  llvm::Function* func = new llvm::Function(
    /*Type=*/ llvm::cast<llvm::FunctionType>(getType(F->getType())),
    /*Linkage=*/ getLinkageTypes(F->getLinkageKind()), 
    /*Name=*/ getLinkageName(F),
    /*Parent=*/ lmod
  );
  funcs[F] = func;
  return func;
}

llvm::GlobalValue::LinkageTypes
LLVMGeneratorPass::getLinkageTypes(LinkageKinds lk)
{
  switch (lk) {
    case hlvm::ExternalLinkage : return llvm::GlobalValue::ExternalLinkage; 
    case hlvm::LinkOnceLinkage : return llvm::GlobalValue::LinkOnceLinkage; 
    case hlvm::WeakLinkage     : return llvm::GlobalValue::WeakLinkage; 
    case hlvm::AppendingLinkage: return llvm::GlobalValue::AppendingLinkage; 
    case hlvm::InternalLinkage : return llvm::GlobalValue::InternalLinkage; 
    default:
      hlvmAssert(!lk && "Bad LinkageKinds");
  }
  return llvm::GlobalValue::InternalLinkage;
}

llvm::Value* 
LLVMGeneratorPass::getReferent(hlvm::ReferenceOp* r)
{
  hlvm::Value* referent = const_cast<hlvm::Value*>(r->getReferent());
  llvm::Value* v = 0;
  if (llvm::isa<AutoVarOp>(referent)) {
    AutoVarMap::const_iterator I = 
      lvars.find(llvm::cast<AutoVarOp>(referent));
    hlvmAssert(I != lvars.end());
    v = I->second;
  } else if (llvm::isa<ConstantValue>(referent)) {
    const hlvm::ConstantValue* cval = llvm::cast<ConstantValue>(referent);
    llvm::Constant* C = getConstant(cval);
    hlvmAssert(C && "Can't generate constant?");
    v = C;
  } else if (llvm::isa<Variable>(referent)) {
    llvm::Value* V = getVariable(llvm::cast<hlvm::Variable>(referent));
    hlvmAssert(V && "Variable not found?");
    v = V;
  } else if (llvm::isa<Function>(referent)) {
    llvm::Function* F = getFunction(llvm::cast<hlvm::Function>(referent));
    hlvmAssert(F && "Function not found?");
    v = F;
  } else if (llvm::isa<Argument>(referent)) {
    ;
  } else
    hlvmDeadCode("Referent not a linkable or autovar?");
  return v;
}

llvm::Value* 
LLVMGeneratorPass::toBoolean(llvm::Value* V)
{
  const llvm::Type* Ty = V->getType();
  if (Ty == llvm::Type::BoolTy)
    return V;

  if (Ty->isInteger() || Ty->isFloatingPoint()) {
    llvm::Constant* CI = llvm::Constant::getNullValue(V->getType());
    return new llvm::SetCondInst(llvm::Instruction::SetNE, V, CI, "i2b", lblk);
  } else if (llvm::isa<llvm::GlobalValue>(V)) {
    // GlobalValues always have non-zero constant address values, so always true
    return llvm::ConstantBool::get(true);
  }
  hlvmAssert(!"Don't know how to convert V into bool");
  return llvm::ConstantBool::get(true);
}

llvm::Value* 
LLVMGeneratorPass::ptr2Value(llvm::Value* V)
{
  if (!llvm::isa<llvm::PointerType>(V->getType()))
    return V;

 // llvm::GetElementPtrInst* GEP = new llvm::GetElementPtrIns(V,
  //    llvm::ConstantInt::get(llvm::Type::UIntTy,0),
   //   llvm::ConstantInt::get(llvm::Type::UIntTy,0),
    //  "ptr2Value", lblk);
  llvm::LoadInst* Load = new llvm::LoadInst(V,"ptr2Value",lblk);
  return Load;
}

void 
LLVMGeneratorPass::pushOperand(llvm::Value* v, const Operator* op)
{
  hlvmAssert(v && "No value to push for operand?");
  hlvmAssert(op && "No operator for value to be pushed?");
  operands[op] = v;
}

llvm::Value*
LLVMGeneratorPass::popOperand(const Operator* op)
{
  hlvmAssert(op && "No operator to pop?");
  OperandMap::iterator I = operands.find(op);
  if (I == operands.end())
    return 0;
  llvm::Value* result = I->second;
  operands.erase(I);
  return result;
}

llvm::Value*
LLVMGeneratorPass::popOperandAsBlock(
  const Operator* op, const std::string& name,
  llvm::BasicBlock*& entry_block, llvm::BasicBlock*& exit_block)
{
  llvm::Value* result = 0;
  llvm::Value* operand = popOperand(op);

  if (const hlvm::Block* B = llvm::dyn_cast<hlvm::Block>(op)) {
    // Get the corresponding entry and exit blocks for B1
    entry_block = enters[B];
    hlvmAssert(entry_block && "No entry block?");
    exit_block = exits[B];
    hlvmAssert(exit_block && "No exit block?");
    // Set the name of the entry block to match its purpose here
    if (entry_block != exit_block) {
      entry_block->setName(name + "_entry");
      exit_block->setName(name + "_exit");
    } else {
      entry_block->setName(name);
    }
    result = operand;
  } else {
    hlvmAssert(operand && "No operand for operator?");
    entry_block = exit_block = new llvm::BasicBlock(name,lfunc); 
    llvm::Value* V = operand;
    hlvmAssert(V && "No value for operand?");
    if (llvm::Instruction* ins = llvm::dyn_cast<llvm::Instruction>(V)) {
      ins->removeFromParent();
      entry_block->getInstList().push_back(ins);
      result = ins;
    } else {
      // Its just a value or a constant or something, just cast it to itself
      // so we can get its value
      result = 
        new llvm::CastInst(V,V->getType(),"",entry_block);
    }
  }

  if (result && result->getType() != llvm::Type::VoidTy)
    result->setName(name + "_rslt");
  return result;
}

llvm::Value*
LLVMGeneratorPass::popOperandAsCondition(
  const Operator* op, const std::string& name,
  llvm::BasicBlock*& entry_block, llvm::BasicBlock*& exit_block)
{
  llvm::Value* result = 
    popOperandAsBlock(op,name+"_cond",entry_block,exit_block);
  hlvmAssert(result);
  hlvmAssert(result->getType() == llvm::Type::BoolTy);

  return result;
}

llvm::BasicBlock*
LLVMGeneratorPass::pushBlock(const std::string& name)
{
  lblk = new llvm::BasicBlock(name,lfunc);
  blocks.push_back(lblk);
  return lblk;
}

llvm::BasicBlock* 
LLVMGeneratorPass::popBlock(llvm::BasicBlock* curBlock)
{
  llvm::BasicBlock* result = blocks.back();
  blocks.pop_back();
  if (blocks.empty())
    lblk = 0;
  else
    lblk = blocks.back();
  return result;
}

llvm::BasicBlock*
LLVMGeneratorPass::newBlock(const std::string& name)
{
  blocks.pop_back();
  lblk = new llvm::BasicBlock(name,lfunc);
  blocks.push_back(lblk);
  return lblk;
}

llvm::AllocaInst* 
LLVMGeneratorPass::getOperatorResult(Operator* op, const std::string& name)
{
  llvm::AllocaInst* result = 0;
  if (!llvm::isa<Block>(op->getParent())) {
    const llvm::Type* Ty = getType(op->getType());
    result = new llvm::AllocaInst(
      /*Ty=*/ Ty,
      /*ArraySize=*/ llvm::ConstantUInt::get(llvm::Type::UIntTy,1),
      /*Name=*/ name + "_var",
      /*InsertAtEnd=*/ &lfunc->front()
    ); 
    new llvm::StoreInst(llvm::Constant::getNullValue(Ty),result,lblk);
  }
  return result;
}

llvm::Value* 
LLVMGeneratorPass::getBlockResult(Block* B)
{
  if (B->getResult() && !lblk->getTerminator()) {
    llvm::Value* result = operands[B];
    if (llvm::isa<llvm::LoadInst>(result))
      result = llvm::cast<llvm::LoadInst>(result)->getOperand(0);
    result = new llvm::LoadInst(result,lblk->getName()+"_result",lblk);
    pushOperand(result,B);
    return result;
  }
  return 0;
}

void 
LLVMGeneratorPass::branchIfNotTerminated(
  llvm::BasicBlock* to, llvm::BasicBlock* from )
{
  if (!from->getTerminator())
    new llvm::BranchInst(to,from);
}

void
LLVMGeneratorPass::startNewFunction(llvm::Function* F)
{
  // Clear the function related variables
  lblk = 0;
  operands.clear();
  enters.clear();
  exits.clear();
  blocks.clear();
  breaks.clear();
  continues.clear();
  lvars.clear();
  // Instantiate an entry block for the alloca'd variables. This block
  // is only used for such variables. By placing the alloca'd variables in
  // the entry block, their allocation is free since the stack pointer 
  // must be adjusted anyway, all that happens is that it gets adjusted
  // by a larger amount. This block is not entered onto the block stack as it
  // has no use but for the alloca'd variables. It is terminated when
  // the function exits
  new llvm::BasicBlock("entry",F);
}

void 
LLVMGeneratorPass::resolveBranches(BranchList& list, llvm::BasicBlock* exit)
{
  for (BranchList::iterator I = list.begin(), E = list.end(); I != E; ++I) {
    (*I)->setOperand(0,exit);
  }
  list.clear();
}

template<> void
LLVMGeneratorPass::gen(AutoVarOp* av)
{
  assert(lblk  != 0 && "Not in block context");
  // Emit an automatic variable. Note that this is inserted into the entry 
  // block, not the current block, for efficiency. This makes automatic 
  // variables zero cost as well as safeguarding against stack growth if the
  // alloca is in a block that is in a loop.
  const llvm::Type* elemType = getType(av->getType());
  llvm::Value* alloca = new llvm::AllocaInst(
    /*Ty=*/ elemType,
    /*ArraySize=*/ llvm::ConstantUInt::get(llvm::Type::UIntTy,1),
    /*Name=*/ av->getName(),
    /*InsertAtEnd=*/ &lfunc->front()
  );
  llvm::Constant* C = 0;
  if (av->hasInitializer())
    C = getConstant(av->getInitializer());
  else
    C = llvm::Constant::getNullValue(elemType);

  if (C) {
    const llvm::Type* CType = C->getType();
    if (CType != elemType) {
      if (llvm::isa<llvm::PointerType>(CType) && 
          llvm::isa<llvm::PointerType>(elemType)) {
        const llvm::Type* CElemType = 
          llvm::cast<llvm::PointerType>(CType)->getElementType();
        const llvm::Type* avElemType = 
            llvm::cast<llvm::PointerType>(elemType)->getElementType();
        if (CElemType != avElemType)
        {
          // We have pointers to different element types. This *can* be okay if
          // we apply conversions.
          if (CElemType == llvm::Type::SByteTy) {
            // The initializer is an sbyte*, which we can convert to either a
            // hlvm_text or an hlvm_buffer.
            if (elemType == get_hlvm_buffer()) {
              // Assign the constant string to the buffer
            } else if (elemType == get_hlvm_text()) {
            }
          }
        }
      }
      hlvmAssert(CType->isLosslesslyConvertibleTo(elemType));
      C = llvm::ConstantExpr::getCast(C,elemType);
    }
    llvm::Value* store = new llvm::StoreInst(C,alloca,lblk);
  }
  pushOperand(alloca,av);
  lvars[av] = alloca;
}


template<> void
LLVMGeneratorPass::gen(NegateOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  hlvmAssert((operand->getType()->isInteger() || 
            operand->getType()->isFloatingPoint()) && 
            "Can't negate non-numeric");
  llvm::BinaryOperator* neg = 
  llvm::BinaryOperator::createNeg(operand,"neg",lblk);
  pushOperand(neg,op);
}

template<> void
LLVMGeneratorPass::gen(ComplementOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  operand = ptr2Value(operand);
  const llvm::Type* lType = operand->getType();
  hlvmAssert(lType->isInteger() && "Can't complement non-integral type");
  llvm::ConstantIntegral* allOnes = 
  llvm::ConstantIntegral::getAllOnesValue(lType);
  llvm::BinaryOperator* cmpl = llvm::BinaryOperator::create(
    llvm::Instruction::Xor,operand,allOnes,"cmpl",lblk);
  pushOperand(cmpl,op);
}

template<> void
LLVMGeneratorPass::gen(PreIncrOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  const llvm::Type* lType = operand->getType();
  hlvmAssert(llvm::isa<llvm::PointerType>(lType));
  const llvm::PointerType* PT = llvm::cast<llvm::PointerType>(lType);
  lType = PT->getElementType();
  llvm::LoadInst* load = new llvm::LoadInst(operand,"preincr",lblk);
  if (lType->isFloatingPoint()) {
    llvm::ConstantFP* one = llvm::ConstantFP::get(lType,1.0);
    llvm::BinaryOperator* add = llvm::BinaryOperator::create(
      llvm::Instruction::Add, load, one, "preincr", lblk);
    new llvm::StoreInst(add,operand,lblk);
    pushOperand(add,op);
  } else if (lType->isInteger()) {
    llvm::ConstantInt* one = llvm::ConstantInt::get(lType,1);
    llvm::BinaryOperator* add = llvm::BinaryOperator::create(
      llvm::Instruction::Add, load, one, "preincr", lblk);
    new llvm::StoreInst(add,operand,lblk);
    pushOperand(add,op);
  } else {
    hlvmAssert(!"PreIncrOp on non-numeric");
  }
}

template<> void
LLVMGeneratorPass::gen(PreDecrOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  const llvm::Type* lType = operand->getType();
  hlvmAssert(llvm::isa<llvm::PointerType>(lType));
  const llvm::PointerType* PT = llvm::cast<llvm::PointerType>(lType);
  lType = PT->getElementType();
  llvm::LoadInst* load = new llvm::LoadInst(operand,"predecr",lblk);
  if (lType->isFloatingPoint()) {
    llvm::ConstantFP* one = llvm::ConstantFP::get(lType,1.0);
    llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
      llvm::Instruction::Sub, load, one, "predecr", lblk);
    new llvm::StoreInst(sub,operand,lblk);
    pushOperand(sub,op);
  } else if (lType->isInteger()) {
    llvm::ConstantInt* one = llvm::ConstantInt::get(lType,1);
    llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
      llvm::Instruction::Sub, load, one, "predecr", lblk);
    new llvm::StoreInst(sub,operand,lblk);
    pushOperand(sub,op);
  } else {
    hlvmAssert(!"PreIncrOp on non-numeric");
  }
}

template<> void
LLVMGeneratorPass::gen(PostIncrOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  const llvm::Type* lType = operand->getType();
  hlvmAssert(llvm::isa<llvm::PointerType>(lType));
  const llvm::PointerType* PT = llvm::cast<llvm::PointerType>(lType);
  lType = PT->getElementType();
  llvm::LoadInst* load = new llvm::LoadInst(operand,"postincr",lblk);
  if (lType->isFloatingPoint()) {
    llvm::ConstantFP* one = llvm::ConstantFP::get(lType,1.0);
    llvm::BinaryOperator* add = llvm::BinaryOperator::create(
      llvm::Instruction::Add, load, one, "postincr", lblk);
    new llvm::StoreInst(add,operand,lblk);
    pushOperand(load,op);
  } else if (lType->isInteger()) {
    llvm::ConstantInt* one = llvm::ConstantInt::get(lType,1);
    llvm::BinaryOperator* add = llvm::BinaryOperator::create(
      llvm::Instruction::Add, load, one, "postincr", lblk);
    new llvm::StoreInst(add,operand,lblk);
    pushOperand(load,op);
  } else {
    hlvmAssert(!"PostDecrOp on non-numeric");
  }
}

template<> void
LLVMGeneratorPass::gen(PostDecrOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  const llvm::Type* lType = operand->getType();
  hlvmAssert(llvm::isa<llvm::PointerType>(lType));
  const llvm::PointerType* PT = llvm::cast<llvm::PointerType>(lType);
  lType = PT->getElementType();
  llvm::LoadInst* load = new llvm::LoadInst(operand,"postdecr",lblk);
  if (lType->isFloatingPoint()) {
    llvm::ConstantFP* one = llvm::ConstantFP::get(lType,1.0);
    llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
      llvm::Instruction::Sub, load, one, "postdecr", lblk);
    new llvm::StoreInst(sub,operand,lblk);
    pushOperand(load,op);
  } else if (lType->isInteger()) {
    llvm::ConstantInt* one = llvm::ConstantInt::get(lType,1);
    llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
      llvm::Instruction::Sub, load, one, "postdecr", lblk);
    new llvm::StoreInst(sub,operand,lblk);
    pushOperand(load,op);
  } else {
    hlvmAssert(!"PostDecrOp on non-numeric");
  }
}

template<> void
LLVMGeneratorPass::gen(AddOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* add = llvm::BinaryOperator::create(
  llvm::Instruction::Add, op1, op2, "add", lblk);
  pushOperand(add,op);
}

template<> void
LLVMGeneratorPass::gen(SubtractOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
  llvm::Instruction::Sub, op1, op2, "add", lblk);
  pushOperand(sub,op);
}

template<> void
LLVMGeneratorPass::gen(MultiplyOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* mul = llvm::BinaryOperator::create(
  llvm::Instruction::Mul, op1, op2, "mul", lblk);
  pushOperand(mul,op);
}

template<> void
LLVMGeneratorPass::gen(DivideOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* div = llvm::BinaryOperator::create(
  llvm::Instruction::Div, op1, op2, "div", lblk);
  pushOperand(div,op);
}

template<> void
LLVMGeneratorPass::gen(ModuloOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* rem = llvm::BinaryOperator::create(
  llvm::Instruction::Rem, op1, op2, "mod", lblk);
  pushOperand(rem,op);
}

template<> void
LLVMGeneratorPass::gen(BAndOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* band = llvm::BinaryOperator::create(
  llvm::Instruction::And, op1, op2, "band", lblk);
  pushOperand(band,op);
}

template<> void
LLVMGeneratorPass::gen(BOrOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* bor = llvm::BinaryOperator::create(
  llvm::Instruction::Or, op1, op2, "bor", lblk);
  pushOperand(bor,op);
}

template<> void
LLVMGeneratorPass::gen(BXorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* Xor = llvm::BinaryOperator::create(
  llvm::Instruction::Xor, op1, op2, "bxor", lblk);
  pushOperand(Xor,op);
}

template<> void
LLVMGeneratorPass::gen(BNorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* bor = llvm::BinaryOperator::create(
  llvm::Instruction::Or, op1, op2, "bnor", lblk);
  llvm::BinaryOperator* nor = llvm::BinaryOperator::createNot(bor,"bnor",lblk);
  pushOperand(nor,op);
}

template<> void
LLVMGeneratorPass::gen(NotOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  llvm::Value* b1 = toBoolean(op1);
  llvm::BinaryOperator* Not = llvm::BinaryOperator::createNot(b1,"not",lblk);
  pushOperand(Not,op);
}

template<> void
LLVMGeneratorPass::gen(AndOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* And = llvm::BinaryOperator::create(
  llvm::Instruction::And, b1, b2, "and", lblk);
  pushOperand(And,op);
}

template<> void
LLVMGeneratorPass::gen(OrOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* Or = llvm::BinaryOperator::create(
  llvm::Instruction::Or, b1, b2, "or", lblk);
  pushOperand(Or,op);
}

template<> void
LLVMGeneratorPass::gen(NorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* Or = llvm::BinaryOperator::create(
  llvm::Instruction::Or, b1, b2, "nor", lblk);
  llvm::BinaryOperator* Nor = llvm::BinaryOperator::createNot(Or,"nor",lblk);
  pushOperand(Nor,op);
}

template<> void
LLVMGeneratorPass::gen(XorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* Xor = llvm::BinaryOperator::create(
  llvm::Instruction::Xor, b1, b2, "xor", lblk);
  pushOperand(Xor,op);
}

template<> void
LLVMGeneratorPass::gen(EqualityOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::SetCondInst* SCI = 
  new llvm::SetCondInst(llvm::Instruction::SetEQ, op1,op2,"eq",lblk);
  pushOperand(SCI,op);
}

template<> void
LLVMGeneratorPass::gen(InequalityOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::SetCondInst* SCI = 
  new llvm::SetCondInst(llvm::Instruction::SetNE, op1,op2,"ne",lblk);
  pushOperand(SCI,op);
}

template<> void
LLVMGeneratorPass::gen(LessThanOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  llvm::SetCondInst* SCI = 
  new llvm::SetCondInst(llvm::Instruction::SetLT, op1,op2,"lt",lblk);
  pushOperand(SCI,op);
}

template<> void
LLVMGeneratorPass::gen(GreaterThanOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  llvm::SetCondInst* SCI = 
  new llvm::SetCondInst(llvm::Instruction::SetGT, op1,op2,"gt",lblk);
  pushOperand(SCI,op);
}

template<> void
LLVMGeneratorPass::gen(GreaterEqualOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  llvm::SetCondInst* SCI = 
  new llvm::SetCondInst(llvm::Instruction::SetGE, op1,op2,"ge",lblk);
  pushOperand(SCI,op);
}

template<> void
LLVMGeneratorPass::gen(LessEqualOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  llvm::SetCondInst* SCI = 
  new llvm::SetCondInst(llvm::Instruction::SetLE, op1,op2,"le",lblk);
  pushOperand(SCI,op);
}

template<> void
LLVMGeneratorPass::gen(SelectOp* op)
{
  // If none of the operands are blocks then we can use LLVM's select
  // instruction to just switch out the result
  if (!llvm::isa<Block>(op->getOperand(0)) &&
    !llvm::isa<Block>(op->getOperand(1)) &&
    !llvm::isa<Block>(op->getOperand(2)))
  {
    // Since HLVM only places on the operand stack things that are of LLVM
    // first class type, we are safe to use select operator here.
    llvm::Value* op1 = popOperand(op->getOperand(0)); 
    llvm::Value* op2 = popOperand(op->getOperand(1)); 
    llvm::Value* op3 = popOperand(op->getOperand(2)); 
    hlvmAssert(op1->getType() == llvm::Type::BoolTy);
    hlvmAssert(op2->getType()->isFirstClassType());
    hlvmAssert(op3->getType()->isFirstClassType());
    pushOperand(new llvm::SelectInst(op1,op2,op3,"select",lblk),op);
    return;
  }

  // Using the LLVM SelectInst won't work.  We must get each operand as a block 
  // and use a branch instruction instead.

  // Get the result of the select operator
  llvm::AllocaInst* select_result = getOperatorResult(op,"select_result");

  // Get the condition block
  llvm::BasicBlock* cond_entry, *cond_exit;
  llvm::Value* op1 = 
  popOperandAsCondition(op->getOperand(0),"select",cond_entry,cond_exit);

  // Branch the current block into the condition block
  new llvm::BranchInst(cond_entry,lblk);

  // Get the true case
  llvm::BasicBlock *true_entry, *true_exit;
  llvm::Value* op2 = 
    popOperandAsBlock(op->getOperand(1),"select_true",true_entry,true_exit); 

  if (select_result && op2)
    new llvm::StoreInst(op2,select_result,true_exit);

  // Get the false case
  llvm::BasicBlock *false_entry, *false_exit;
  llvm::Value* op3 = 
    popOperandAsBlock(op->getOperand(2),"select_false",false_entry,false_exit); 

  if (select_result && op3)
    new llvm::StoreInst(op3,select_result,false_exit);

  // Create the exit block
  llvm::BasicBlock* select_exit = newBlock("select_exit");

  // Branch the the true and false cases to the exit
  branchIfNotTerminated(select_exit,true_exit);
  branchIfNotTerminated(select_exit,false_exit);

  // Finally, install the conditional branch
  new llvm::BranchInst(true_entry,false_entry,op1,cond_exit);

  if (select_result)
    pushOperand(new llvm::LoadInst(select_result,"select_result",select_exit),op);
}

template<> void
LLVMGeneratorPass::gen(SwitchOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
}

template<> void
LLVMGeneratorPass::gen(WhileOp* op)
{
  // Get the result of this while block, if there should be one
  llvm::AllocaInst* while_result = getOperatorResult(op,"while_result");

  // Get the condition block
  llvm::BasicBlock* cond_entry, *cond_exit;
  llvm::Value* op1 = 
    popOperandAsCondition(op->getOperand(0),"while",cond_entry,cond_exit);

  // Branch the current block into the condition block
  new llvm::BranchInst(cond_entry,lblk);

  // Get the while loop's body
  llvm::BasicBlock *body_entry, *body_exit;
  llvm::Value* op2 = 
    popOperandAsBlock(op->getOperand(1),"while_body",body_entry,body_exit); 

  // Save the result of the while body, if there should be one
  if (while_result && op2)
    new llvm::StoreInst(op2,while_result,body_exit);

  // Create the exit block
  llvm::BasicBlock* while_exit = newBlock("while_exit");

  // Branch the the body block back to the condition branch
  branchIfNotTerminated(cond_entry,body_exit);

  // Finally, install the conditional branch into the branch block
  new llvm::BranchInst(body_entry,while_exit,op1,cond_exit);

  // If there's a result, push it now
  if (while_result)
    pushOperand(new llvm::LoadInst(while_result,"while_result",while_exit),op);

  // Fix up any break or continue operators
  resolveBranches(breaks,while_exit);
  resolveBranches(continues,cond_entry);
}

template<> void
LLVMGeneratorPass::gen(UnlessOp* op)
{
  // Get the result of this unless block, if there should be one
  llvm::AllocaInst* unless_result = getOperatorResult(op,"unless_result");

  // Get the condition block
  llvm::BasicBlock* cond_entry, *cond_exit;
  llvm::Value* op1 = 
    popOperandAsCondition(op->getOperand(0),"unless",cond_entry,cond_exit);

  // Branch the current block into the condition block
  new llvm::BranchInst(cond_entry,lblk);

  // Get the unless block's body
  llvm::BasicBlock *body_entry, *body_exit;
  llvm::Value* op2 = 
    popOperandAsBlock(op->getOperand(1),"unless_body",body_entry,body_exit); 

  // Save the result of the unless body, if there should be one
  if (unless_result && op2)
    new llvm::StoreInst(op2,unless_result,body_exit);

  // Create the exit block
  llvm::BasicBlock* unless_exit = newBlock("unless_exit");

  // Branch the the body block back to the condition branch
  branchIfNotTerminated(cond_entry,body_exit);

  // Finally, install the conditional branch into the branch block
  new llvm::BranchInst(unless_exit,body_entry,op1,cond_exit);

  // If there's a result, push it now
  if (unless_result)
    pushOperand(
      new llvm::LoadInst(unless_result,"unless_result",unless_exit),op);

  // Fix up any break or continue operators
  resolveBranches(breaks,unless_exit);
  resolveBranches(continues,cond_entry);
}

template<> void
LLVMGeneratorPass::gen(UntilOp* op)
{
  // Get the result of this until block, if there should be one
  llvm::AllocaInst* until_result = getOperatorResult(op,"until_result");

  // Get the condition block
  llvm::BasicBlock* cond_entry, *cond_exit;
  llvm::Value* op2 = 
    popOperandAsCondition(op->getOperand(1),"until",cond_entry,cond_exit);

  // Get the until block's body
  llvm::BasicBlock *body_entry, *body_exit;
  llvm::Value* op1 = 
    popOperandAsBlock(op->getOperand(0),"until_body",body_entry,body_exit); 

  // Save the result of the until body, if there should be one
  if (until_result && op1)
    new llvm::StoreInst(op1,until_result,body_exit);

  // Branch the current block into the body block
  new llvm::BranchInst(body_entry,lblk);

  // Branch the body block to the condition block
  branchIfNotTerminated(cond_entry,body_exit);

  // Create the exit block
  llvm::BasicBlock* until_exit = newBlock("until_exit");

  // Finally, install the conditional branch into condition block
  new llvm::BranchInst(until_exit,body_entry,op2,cond_exit);

  // If there's a result, push it now
  if (until_result)
    pushOperand(new llvm::LoadInst(until_result,"until_result",until_exit),op);

  // Fix up any break or continue operators
  resolveBranches(breaks,until_exit);
  resolveBranches(continues,cond_entry);
}

template<> void
LLVMGeneratorPass::gen(LoopOp* op)
{
  // Get the result of this loop block, if there should be one
  llvm::AllocaInst* loop_result = getOperatorResult(op,"loop_result");

  // Get the start condition block
  llvm::BasicBlock* start_cond_entry, *start_cond_exit;
  llvm::Value* op1 = 
    popOperandAsCondition(op->getOperand(0),"loop_start",
      start_cond_entry,start_cond_exit);

  // Branch the current block into the start condition block
  new llvm::BranchInst(start_cond_entry,lblk);

  // Get the loop body
  llvm::BasicBlock *body_entry, *body_exit;
  llvm::Value* op2 = 
    popOperandAsBlock(op->getOperand(1),"loop_body",body_entry,body_exit); 

  // Save the result of the loop body, if there should be one
  if (loop_result && op2)
    new llvm::StoreInst(op2,loop_result,body_exit);

  // Get the end condition block
  llvm::BasicBlock* end_cond_entry, *end_cond_exit;
  llvm::Value* op3 = 
    popOperandAsCondition(op->getOperand(2),"loop_end",
      end_cond_entry,end_cond_exit);

  // Branch the loop body to the end condition block
  branchIfNotTerminated(end_cond_entry,body_exit);

  // Create the exit block
  llvm::BasicBlock* loop_exit = newBlock("loop_exit");

  // Install the conditional branches for start and end condition blocks
  new llvm::BranchInst(body_entry,loop_exit,op1,start_cond_exit);
  new llvm::BranchInst(start_cond_entry,loop_exit,op3,end_cond_exit);

  // If there's a result, push it now
  if (loop_result)
    pushOperand(new llvm::LoadInst(loop_result,"loop_result",loop_exit),op);

  // Fix up any break or continue operators
  resolveBranches(breaks,loop_exit);
  resolveBranches(continues,end_cond_entry);
}

template<> void
LLVMGeneratorPass::gen(BreakOp* op)
{
  // Make sure the block result is stored
  Block* B = llvm::cast<Block>(op->getContainingBlock());
  getBlockResult(B);

  // Just push a place-holder branch onto the breaks list so it can
  // be fixed up later once we know the destination
  llvm::BranchInst* brnch = new llvm::BranchInst(lblk,lblk);
  breaks.push_back(brnch);
  pushOperand(brnch,op);
}

template<> void
LLVMGeneratorPass::gen(ContinueOp* op)
{
  // Make sure the block result is stored
  Block* B = llvm::cast<Block>(op->getParent());
  getBlockResult(B);

  // Just push a place-holder branch onto the continues list so it can
  // be fixed up later once we know the destination
  llvm::BranchInst* brnch = new llvm::BranchInst(lblk,lblk);
  continues.push_back(brnch);
  pushOperand(brnch,op);
}

template<> void
LLVMGeneratorPass::gen(ResultOp* r)
{
  // Get the result operand
  llvm::Value* result = popOperand(r->getOperand(0));
  // Get the block this result applies to
  hlvm::Block* B = llvm::cast<hlvm::Block>(r->getParent());
  // Generate a store into the location set up by the block for its result
  llvm::Value* instr = operands[B];
  if (llvm::isa<llvm::LoadInst>(instr))
    instr = llvm::cast<llvm::LoadInst>(instr)->getOperand(0);
  new llvm::StoreInst(result,instr,lblk);
}

template<> void
LLVMGeneratorPass::gen(ReturnOp* r)
{
  // Initialize the result value. A null Value* indicates no result.
  llvm::Value* result = 0;

  // If this function returns a result then we need a return value
  if (lfunc->getReturnType() != llvm::Type::VoidTy) {
    result = operands[function->getBlock()];
    const llvm::Type* resultTy = result->getType();
    if (const llvm::PointerType* PTy = 
        llvm::dyn_cast<llvm::PointerType>(resultTy)) {
      hlvmAssert(PTy->getElementType() == lfunc->getReturnType());
      result = new llvm::LoadInst(result,lblk->getName()+"_result",lblk);
    } else if (resultTy != lfunc->getReturnType()) {
      result = new llvm::CastInst(result,lfunc->getReturnType(),
        lblk->getName()+"_result",lblk);
    }
    hlvmAssert(result && "No result for function");
  }

  // RetInst is never the operand of another instruction because it is
  // a terminator and cannot return a value. Consequently, we don't push it
  // on the operand stack.
  new llvm::ReturnInst(result,lblk);
}

template<> void
LLVMGeneratorPass::gen(CallOp* co)
{
  hlvm::Function* hFunc = co->getCalledFunction();
  const SignatureType* sigTy = hFunc->getSignature();
  // Set up the loop
  CallOp::iterator I = co->begin();
  CallOp::iterator E = co->end();

  // Get the function (first operand)
  llvm::Value* funcToCall = popOperand(*I++);
  hlvmAssert(funcToCall && "No function to call?");
  hlvmAssert(llvm::isa<llvm::Function>(funcToCall));

  // Get the function call arguments
  std::vector<llvm::Value*> args;
  for ( ; I != E; ++I ) {
    llvm::Value* arg = popOperand(*I);
    hlvmAssert(arg && "No argument for CallOp?");
    args.push_back(arg);
  }

  // Make sure we have sanity with varargs functions
  hlvmAssert(sigTy->isVarArgs() || args.size() == sigTy->size());
  hlvmAssert(!sigTy->isVarArgs() || args.size() >= sigTy->size());

  // convert to function type
  llvm::Function* F = const_cast<llvm::Function*>(
    llvm::cast<llvm::Function>(funcToCall));

  // Make the call
  pushOperand(new llvm::CallInst(F,args,"call_" + hFunc->getName(),lblk),co);
}

template<> void
LLVMGeneratorPass::gen(StoreOp* s)
{
  llvm::Value* location = popOperand(s->getOperand(0));
  llvm::Value* value =    popOperand(s->getOperand(1));
  // We don't push the StoreInst as an operand because it has no value and
  // therefore cannot be an operand.
  new llvm::StoreInst(value,location,lblk);
}

template<> void
LLVMGeneratorPass::gen(LoadOp* l)
{
  llvm::Value* location = popOperand(l->getOperand(0));
  pushOperand(new llvm::LoadInst(location,location->getName(),lblk),l);
}

template<> void
LLVMGeneratorPass::gen(ReferenceOp* r)
{
  llvm::Value* referent = getReferent(r);
  pushOperand(referent,r);
}

template<> void
LLVMGeneratorPass::gen(IndexOp* r)
{
}

template<> void
LLVMGeneratorPass::gen(OpenOp* o)
{
  llvm::Value* strm = popOperand(o->getOperand(0));
  std::vector<llvm::Value*> args;
  if (const llvm::PointerType* PT = 
      llvm::dyn_cast<llvm::PointerType>(strm->getType())) {
  const llvm::Type* Ty = PT->getElementType();
  if (Ty == llvm::Type::SByteTy) {
    args.push_back(strm);
  } else if (llvm::isa<llvm::ArrayType>(Ty) && 
             llvm::cast<llvm::ArrayType>(Ty)->getElementType() == 
             llvm::Type::SByteTy) {
    std::vector<llvm::Value*> indices;
    indices.push_back(llvm::Constant::getNullValue(llvm::Type::UIntTy));
    indices.push_back(llvm::Constant::getNullValue(llvm::Type::UIntTy));
    llvm::GetElementPtrInst* gep = 
      new llvm::GetElementPtrInst(strm,indices,"",lblk);
    args.push_back(gep);
  } else
    hlvmAssert(!"Array element type is not SByteTy");
  } else
    hlvmAssert(!"OpenOp parameter is not a pointer");
  llvm::CallInst* ci = new 
  llvm::CallInst(get_hlvm_stream_open(), args, "open", lblk);
  pushOperand(ci,o);
}

template<> void
LLVMGeneratorPass::gen(WriteOp* o)
{
  llvm::Value* strm = popOperand(o->getOperand(0));
  llvm::Value* arg2 = popOperand(o->getOperand(1));
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  args.push_back(arg2);
  if (llvm::isa<llvm::PointerType>(arg2->getType()))
  if (llvm::cast<llvm::PointerType>(arg2->getType())->getElementType() ==
      llvm::Type::SByteTy)
    pushOperand(
      new llvm::CallInst(get_hlvm_stream_write_string(), args, "", lblk),o);
  if (arg2->getType() == get_hlvm_text())
    pushOperand(
      new llvm::CallInst(get_hlvm_stream_write_text(), args, "", lblk),o);
  else if (arg2->getType() == get_hlvm_buffer())
    pushOperand(
      new llvm::CallInst(get_hlvm_stream_write_buffer(), args, "write",lblk),o);
}

template<> void
LLVMGeneratorPass::gen(ReadOp* o)
{
  llvm::Value* strm = popOperand(o->getOperand(0));
  llvm::Value* arg2 = popOperand(o->getOperand(1));
  llvm::Value* arg3 = popOperand(o->getOperand(2));
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  args.push_back(arg2);
  args.push_back(arg3);
  llvm::CallInst* ci = 
    new llvm::CallInst(get_hlvm_stream_read(), args, "read", lblk);
  pushOperand(ci,o);
}

template<> void
LLVMGeneratorPass::gen(CloseOp* o)
{
  llvm::Value* strm = popOperand(o->getOperand(0));
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  llvm::CallInst* ci = 
    new llvm::CallInst(get_hlvm_stream_close(), args, "", lblk);
  pushOperand(ci,o);
}

void 
LLVMGeneratorPass::genProgramLinkage()
{
  // Short circuit if there's nothing to do
  if (progs.empty())
    return;

  // Define the type of the array elements (a structure with a pointer to
  // a string and a pointer to the function).
  std::vector<const llvm::Type*> Fields;
  Fields.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
  Fields.push_back(llvm::PointerType::get(get_hlvm_program_signature()));
  llvm::StructType* entry_elem_type = llvm::StructType::get(Fields);

  // Define the type of the array for the entry points
  llvm::ArrayType* entry_points_type = 
    llvm::ArrayType::get(entry_elem_type,progs.size());

  // Create a vector to hold the entry elements as they are created.
  std::vector<llvm::Constant*> entry_points_items;

  for (std::vector<llvm::Function*>::iterator I = progs.begin(), 
     E = progs.end(); I != E; ++I )
  {
    const std::string& funcName = (*I)->getName();
    // Get a constant for the name of the entry point (char array)
    llvm::Constant* name_val = llvm::ConstantArray::get(funcName,true);

    // Create a constant global variable to hold the name of the program.
    llvm::GlobalVariable* name = new llvm::GlobalVariable(
      /*Type=*/name_val->getType(),
      /*isConst=*/true,
      /*Linkage=*/llvm::GlobalValue::InternalLinkage, 
      /*Initializer=*/name_val, 
      /*name=*/"", 
      /*InsertInto=*/lmod
    );

    llvm::Constant* index = llvm::ConstantExpr::getPtrPtrFromArrayPtr(name);

    // Get a constant structure for the entry containing the name and pointer
    // to the function.
    std::vector<llvm::Constant*> items;
    items.push_back(index);
    items.push_back(*I);
    llvm::Constant* entry = llvm::ConstantStruct::get(entry_elem_type,items);

    // Save the entry into the list of entry point items
    entry_points_items.push_back(entry);
  }

  // Create a constant array to initialize the entry_points
  llvm::Constant* entry_points_initializer = llvm::ConstantArray::get(
    entry_points_type,entry_points_items);

  // Now get the GlobalVariable
  llvm::GlobalVariable* entry_points = new llvm::GlobalVariable(
    /*Type=*/entry_points_type,
    /*isConstant=*/true,
    /*Linkage=*/llvm::GlobalValue::AppendingLinkage,
    /*Initializer=*/entry_points_initializer,
    /*Name=*/"hlvm_programs",
    /*Parent=*/lmod
  );
}

void
LLVMGeneratorPass::handleInitialize(AST* tree)
{
  // Nothing to do
}

void
LLVMGeneratorPass::handle(Node* n,Pass::TraversalKinds mode)
{
  if (mode == Pass::PreOrderTraversal) {
    // We process container nodes here (preorder) to ensure that we create the
    // container that is being asked for.
    switch (n->getID()) 
    {
      case BundleID:
      {
        lmod = new llvm::Module(llvm::cast<Bundle>(n)->getName());
        modules.push_back(lmod);
        lvars.clear();
        gvars.clear();
        funcs.clear();
        break;
      }
      case ConstantBooleanID:       
        getConstant(llvm::cast<ConstantBoolean>(n));
        break;
      case ConstantIntegerID:       
        getConstant(llvm::cast<ConstantInteger>(n));
        break;
      case ConstantRealID:          
        getConstant(llvm::cast<ConstantReal>(n));
        break;
      case ConstantStringID:        
        getConstant(llvm::cast<ConstantString>(n));
        break;
      case VariableID:              
        getVariable(llvm::cast<Variable>(n)); 
        break;
      case FunctionID:
      {
        // Get/Create the function
        function = llvm::cast<hlvm::Function>(n);
        lfunc = getFunction(function);
        startNewFunction(lfunc);
        break;
      }
      case ProgramID:
      {
        // Create a new function for the program based on the signature
        function = llvm::cast<hlvm::Program>(n);
        lfunc = new llvm::Function(
            /*Type=*/ get_hlvm_program_signature(),
            /*Linkage=*/llvm::GlobalValue::InternalLinkage,
            /*Name=*/ getLinkageName(function),
            /*Module=*/ lmod
        );
        startNewFunction(lfunc);
        // Save the program so it can be generated into the list of program 
        // entry points.
        progs.push_back(lfunc);
        break;
      }
      case BlockID:
      {
        Block* B = llvm::cast<Block>(n);
        std::string name = B->getLabel().empty() ? "block" : B->getLabel();
        enters[B] = pushBlock(name);
        if (B->getResult()) {
          const llvm::Type* Ty = getType(B->getType());
          llvm::AllocaInst* result = new llvm::AllocaInst(
            /*Ty=*/ Ty,
            /*ArraySize=*/ llvm::ConstantUInt::get(llvm::Type::UIntTy,1),
            /*Name=*/ name + "_var",
            /*InsertAtEnd=*/ &lfunc->front()
          ); 
          // Initialize the autovar to null
          new llvm::StoreInst(llvm::Constant::getNullValue(Ty),result,lblk);
          pushOperand(result,B); 
        }
        break;
      }
      default:
        break;
    }
  } else {
    // We process non-container nodes and operators. Operators are done
    // post-order because we want their operands to be constructed first.
    switch (n->getID()) 
    {
      case AnyTypeID:
      case StringTypeID:
      case BooleanTypeID:
      case CharacterTypeID:
      case IntegerTypeID:
      case RangeTypeID:
      case EnumerationTypeID:
      case RealTypeID:
      case OctetTypeID:
      case PointerTypeID:
      case ArrayTypeID:
      case VectorTypeID:
      case StructureTypeID:
      case SignatureTypeID:
      case OpaqueTypeID:
      {
        Type* t = llvm::cast<Type>(n);
        lmod->addTypeName(t->getName(), getType(t));
        break;
      }
      case NegateOpID:              gen(llvm::cast<NegateOp>(n)); break;
      case ComplementOpID:          gen(llvm::cast<ComplementOp>(n)); break;
      case PreIncrOpID:             gen(llvm::cast<PreIncrOp>(n)); break;
      case PreDecrOpID:             gen(llvm::cast<PreDecrOp>(n)); break;
      case PostIncrOpID:            gen(llvm::cast<PostIncrOp>(n)); break;
      case PostDecrOpID:            gen(llvm::cast<PostDecrOp>(n)); break;
      case AddOpID:                 gen(llvm::cast<AddOp>(n)); break;
      case SubtractOpID:            gen(llvm::cast<SubtractOp>(n)); break;
      case MultiplyOpID:            gen(llvm::cast<MultiplyOp>(n)); break;
      case DivideOpID:              gen(llvm::cast<DivideOp>(n)); break;
      case ModuloOpID:              gen(llvm::cast<ModuloOp>(n)); break;
      case BAndOpID:                gen(llvm::cast<BAndOp>(n)); break;
      case BOrOpID:                 gen(llvm::cast<BOrOp>(n)); break;
      case BXorOpID:                gen(llvm::cast<BXorOp>(n)); break;
      case BNorOpID:                gen(llvm::cast<BNorOp>(n)); break;
      case NotOpID:                 gen(llvm::cast<NotOp>(n)); break;
      case AndOpID:                 gen(llvm::cast<AndOp>(n)); break;
      case OrOpID:                  gen(llvm::cast<OrOp>(n)); break;
      case NorOpID:                 gen(llvm::cast<NorOp>(n)); break;
      case XorOpID:                 gen(llvm::cast<XorOp>(n)); break;
      case EqualityOpID:            gen(llvm::cast<EqualityOp>(n)); break;
      case InequalityOpID:          gen(llvm::cast<InequalityOp>(n)); break;
      case LessThanOpID:            gen(llvm::cast<LessThanOp>(n)); break;
      case GreaterThanOpID:         gen(llvm::cast<GreaterThanOp>(n)); break;
      case GreaterEqualOpID:        gen(llvm::cast<GreaterEqualOp>(n)); break;
      case LessEqualOpID:           gen(llvm::cast<LessEqualOp>(n)); break;
      case SelectOpID:              gen(llvm::cast<SelectOp>(n)); break;
      case SwitchOpID:              gen(llvm::cast<SwitchOp>(n)); break;
      case WhileOpID:               gen(llvm::cast<WhileOp>(n)); break;
      case UnlessOpID:              gen(llvm::cast<UnlessOp>(n)); break;
      case UntilOpID:               gen(llvm::cast<UntilOp>(n)); break;
      case LoopOpID:                gen(llvm::cast<LoopOp>(n)); break;
      case BreakOpID:               gen(llvm::cast<BreakOp>(n)); break;
      case ContinueOpID:            gen(llvm::cast<ContinueOp>(n)); break;
      case ReturnOpID:              gen(llvm::cast<ReturnOp>(n)); break;
      case ResultOpID:              gen(llvm::cast<ResultOp>(n)); break;
      case CallOpID:                gen(llvm::cast<CallOp>(n)); break;
      case LoadOpID:                gen(llvm::cast<LoadOp>(n)); break;
      case StoreOpID:               gen(llvm::cast<StoreOp>(n)); break;
      case ReferenceOpID:           gen(llvm::cast<ReferenceOp>(n)); break;
      case AutoVarOpID:             gen(llvm::cast<AutoVarOp>(n)); break;
      case OpenOpID:                gen(llvm::cast<OpenOp>(n)); break;
      case CloseOpID:               gen(llvm::cast<CloseOp>(n)); break;
      case WriteOpID:               gen(llvm::cast<WriteOp>(n)); break;
      case ReadOpID:                gen(llvm::cast<ReadOp>(n)); break;

      case BundleID:
        genProgramLinkage();
        lmod = 0;
        break;
      case ConstantBooleanID:       
        /* FALL THROUGH */
      case ConstantIntegerID:      
        /* FALL THROUGH */
      case ConstantRealID:        
        /* FALL THROUGH */
      case ConstantStringID:     
        /* FALL THROUGH */
      case VariableID: 
        break;
      case ProgramID:
        /* FALL THROUGH */
      case FunctionID:
        // The entry block was created to hold the automatic variables. We now
        // need to terminate the block by branching it to the first active block
        // in the function.
        new llvm::BranchInst(lfunc->front().getNext(),&lfunc->front());
        function = 0;
        lfunc = 0;
        break;
      case BlockID:
      {
        Block* B = llvm::cast<Block>(n);
        exits[B] = lblk;
        getBlockResult(B);
        popBlock(lblk);
        break;
      }

      // everything else is an error
      default:
        hlvmNotImplemented("Node of unimplemented type");
        break;
    }
  }
}

void
LLVMGeneratorPass::handleTerminate()
{
  // Nothing to do.
}

llvm::Module*
LLVMGeneratorPass::linkModules()
{
  llvm::Linker linker("HLVM",ast->getPublicID(),0);
  for (ModuleList::iterator I = modules.begin(), E = modules.end(); I!=E; ++I) {
    linker.LinkInModule(*I);
  }
  modules.empty(); // LinkInModules destroyed/merged them all
  return linker.releaseModule();
}


llvm::cl::opt<bool>
  NoInline("no-inlining", 
    llvm::cl::desc("Do not run the LLVM inliner pass"));

llvm::cl::opt<bool>
  NoOptimizations("no-optimization",
    llvm::cl::desc("Do not run any LLVM optimization passes"));

void 
getCleanupPasses(llvm::PassManager& PM)
{
  
  PM.add(llvm::createLowerSetJmpPass());          // Lower llvm.setjmp/.longjmp
  PM.add(llvm::createFunctionResolvingPass());    // Resolve (...) functions
  if (NoOptimizations)
    return;

  PM.add(llvm::createRaiseAllocationsPass());     // call %malloc -> malloc inst
  PM.add(llvm::createCFGSimplificationPass());    // Clean up disgusting code
  PM.add(llvm::createPromoteMemoryToRegisterPass());// Kill useless allocas
  PM.add(llvm::createGlobalOptimizerPass());      // Optimize out global vars
  PM.add(llvm::createGlobalDCEPass());            // Remove unused fns and globs
  PM.add(llvm::createIPConstantPropagationPass());// IP Constant Propagation
  PM.add(llvm::createDeadArgEliminationPass());   // Dead argument elimination
  PM.add(llvm::createInstructionCombiningPass()); // Clean up after IPCP & DAE
  PM.add(llvm::createCFGSimplificationPass());    // Clean up after IPCP & DAE
  PM.add(llvm::createPruneEHPass());              // Remove dead EH info
  if (!NoInline)
    PM.add(llvm::createFunctionInliningPass());   // Inline small functions
  PM.add(llvm::createSimplifyLibCallsPass());     // Library Call Optimizations
  PM.add(llvm::createArgumentPromotionPass());    // Scalarize uninlined fn args
  PM.add(llvm::createRaisePointerReferencesPass());// Recover type information
  PM.add(llvm::createTailDuplicationPass());      // Simplify cfg by copying 
  PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
  PM.add(llvm::createScalarReplAggregatesPass()); // Break up aggregate allocas
  PM.add(llvm::createInstructionCombiningPass()); // Combine silly seq's
  PM.add(llvm::createCondPropagationPass());      // Propagate conditionals
  PM.add(llvm::createTailCallEliminationPass());  // Eliminate tail calls
  PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
  PM.add(llvm::createReassociatePass());          // Reassociate expressions
  PM.add(llvm::createLICMPass());                 // Hoist loop invariants
  PM.add(llvm::createLoopUnswitchPass());         // Unswitch loops.
  PM.add(llvm::createInstructionCombiningPass()); // Clean up after LICM/reassoc
  PM.add(llvm::createIndVarSimplifyPass());       // Canonicalize indvars
  PM.add(llvm::createLoopUnrollPass());           // Unroll small loops
  PM.add(llvm::createInstructionCombiningPass()); // Clean up after the unroller
  PM.add(llvm::createLoadValueNumberingPass());   // GVN for load instructions
  PM.add(llvm::createGCSEPass());                 // Remove common subexprs
  PM.add(llvm::createSCCPPass());                 // Constant prop with SCCP

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  PM.add(llvm::createInstructionCombiningPass());
  PM.add(llvm::createCondPropagationPass());      // Propagate conditionals

  PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
  PM.add(llvm::createAggressiveDCEPass());        // SSA based 'Aggressive DCE'
  PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
  PM.add(llvm::createDeadTypeEliminationPass());  // Eliminate dead types
  PM.add(llvm::createConstantMergePass());        // Merge dup global constants
}

}

bool
hlvm::generateBytecode(AST* tree, std::ostream& output, bool verify)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  delete PM;
  llvm::Module* mod = genPass.linkModules();
  bool result = false;
  llvm::PassManager Passes;
  Passes.add(new llvm::TargetData(mod));
  if (verify)
    Passes.add(llvm::createVerifierPass());
  getCleanupPasses(Passes);
  if (verify)
    Passes.add(llvm::createVerifierPass());
  Passes.run(*mod);
  llvm::WriteBytecodeToFile(mod, output, /*compress= */ true);
  result = true;
  delete mod;
  return result;
}

bool
hlvm::generateAssembly(AST* tree, std::ostream& output, bool verify)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  delete PM;
  llvm::Module* mod = genPass.linkModules();
  bool result = false;
  llvm::PassManager lPM;
  lPM.add(new llvm::TargetData(mod));
  if (verify)
    lPM.add(llvm::createVerifierPass());
  getCleanupPasses(lPM);
  if (verify)
    lPM.add(llvm::createVerifierPass());
  lPM.add(new llvm::PrintModulePass(&output));
  lPM.run(*mod);
  result = true;
  delete mod;
  return result;
}

llvm::Module* 
hlvm::generateModule(AST* tree, bool verify)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  delete PM;
  llvm::Module* mod = genPass.linkModules();
  llvm::PassManager lPM;
  lPM.add(new llvm::TargetData(mod));
  if (verify)
    lPM.add(llvm::createVerifierPass());
  getCleanupPasses(lPM);
  if (verify)
    lPM.add(llvm::createVerifierPass());
  lPM.run(*mod);
  return mod;
}

