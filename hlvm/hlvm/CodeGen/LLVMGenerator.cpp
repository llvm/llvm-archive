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
#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Instructions.h>
#include <llvm/DerivedTypes.h>
#include <llvm/TypeSymbolTable.h>
#include <llvm/ValueSymbolTable.h>
#include <llvm/Constants.h>
#include <llvm/CallingConv.h>
#include <llvm/Linker.h>
#include <llvm/Bytecode/Writer.h>
#include <llvm/PassManager.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Analysis/Verifier.h>

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
  typedef std::vector<llvm::Value*> OperandList;
  typedef std::map<const hlvm::Variable*,llvm::Value*> VariableDictionary;
  typedef std::map<const hlvm::AutoVarOp*,llvm::Value*> AutoVarDictionary;
  typedef std::map<const hlvm::ConstantValue*,llvm::Constant*> ConstantDictionary;
  typedef std::map<const hlvm::Function*,llvm::Function*> FunctionDictionary;
  ModuleList modules;        ///< The list of modules we construct
  llvm::Module*     lmod;    ///< The current module we're generation 
  llvm::Function*   lfunc;   ///< The current LLVM function we're generating 
  llvm::BasicBlock* lblk;    ///< The current LLVM block we're generating
  llvm::BasicBlock::InstListType linst; 
  OperandList lops;          ///< The current list of instruction operands
  VariableDictionary gvars;
  AutoVarDictionary lvars;
  llvm::TypeSymbolTable ltypes; ///< The cached LLVM types we've generated
  ConstantDictionary consts; ///< The cached LLVM constants we've generated
  FunctionDictionary funcs;  ///< The cached LLVM constants we've generated
    ///< The current LLVM instructions we're generating
  const AST* ast;            ///< The current Tree we're traversing
  const Bundle* bundle;      ///< The current Bundle we're traversing
  const hlvm::Function* function;  ///< The current Function we're traversing
  const Block* block;        ///< The current Block we're traversing
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
      modules(), lmod(0), lfunc(0), lblk(0), linst(), lops(), 
      gvars(), lvars(), ltypes(), consts(),
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
  inline llvm::Value* toBoolean(llvm::Value* op);
  inline llvm::Value* ptr2Value(llvm::Value* op);
  inline llvm::Value* coerce(llvm::Value* op);

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
    case VoidTypeID:               result = llvm::Type::VoidTy; break;
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
    case AliasTypeID:
      result = getType(llvm::cast<AliasType>(ty)->getElementType());
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
        Fields.push_back(getType((*I)->getElementType()));
      result = llvm::StructType::get(Fields);
      break;
    }
    case SignatureTypeID:
    {
      std::vector<const llvm::Type*> params;
      const SignatureType* st = llvm::cast<SignatureType>(ty);
      for (SignatureType::const_iterator I = st->begin(), E = st->end(); 
           I != E; ++I)
        params.push_back(getType(*I));
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
  ConstantDictionary::iterator I = 
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
  VariableDictionary::iterator I = 
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
  FunctionDictionary::iterator I = funcs.find(const_cast<hlvm::Function*>(F));
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

template<> void 
LLVMGeneratorPass::gen(AliasType* t)
{
  lmod->addTypeName(t->getName(), getType(t->getElementType()));
}

template<> void 
LLVMGeneratorPass::gen(AnyType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(StringType* s)
{
  lmod->addTypeName(s->getName(), getType(s));
}

template<> void
LLVMGeneratorPass::gen(BooleanType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen(CharacterType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen(IntegerType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen(RangeType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(EnumerationType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen(RealType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen(OctetType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen(VoidType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(hlvm::PointerType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(hlvm::ArrayType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(VectorType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(StructureType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(SignatureType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<hlvm::OpaqueType>(hlvm::OpaqueType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen(ConstantInteger* i)
{
  llvm::Constant* C = getConstant(i);
}

template<> void
LLVMGeneratorPass::gen(ConstantBoolean* b)
{
  getConstant(b);
}

template<> void
LLVMGeneratorPass::gen(ConstantReal* r)
{
  getConstant(r);
}

template<> void
LLVMGeneratorPass::gen(ConstantString* t)
{
  getConstant(t);
}

template<> void
LLVMGeneratorPass::gen(AutoVarOp* av)
{
  assert(lblk  != 0 && "Not in block context");
  // emit a stack variable
  const llvm::Type* elemType = getType(av->getType());
  llvm::Value* alloca = new llvm::AllocaInst(
      /*Ty=*/ elemType,
      /*ArraySize=*/ llvm::ConstantUInt::get(llvm::Type::UIntTy,1),
      /*Name=*/ av->getName(),
      /*InsertAtEnd=*/ lblk
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
    llvm::Value* store = new llvm::StoreInst(C,alloca,"",lblk);
  }
  lops.push_back(alloca);
  lvars[av] = alloca;
}

template<> void
LLVMGeneratorPass::gen(Variable* v)
{
  getVariable(v);
}

template<> void 
LLVMGeneratorPass::gen(Block* b)
{
  lblk = new llvm::BasicBlock(b->getLabel(),lfunc,0);
}

template<> void
LLVMGeneratorPass::gen(NegateOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for NegateOp");
  llvm::Value* operand = lops.back(); lops.pop_back();
  hlvmAssert((operand->getType()->isInteger() || 
              operand->getType()->isFloatingPoint()) && 
              "Can't negate non-numeric");
  llvm::BinaryOperator* neg = 
    llvm::BinaryOperator::createNeg(operand,"neg",lblk);
  lops.push_back(neg);
}

template<> void
LLVMGeneratorPass::gen(ComplementOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for ComplementOp");
  llvm::Value* operand = lops.back(); lops.pop_back();
  operand = ptr2Value(operand);
  const llvm::Type* lType = operand->getType();
  hlvmAssert(lType->isInteger() && "Can't complement non-integral type");
  llvm::ConstantIntegral* allOnes = 
    llvm::ConstantIntegral::getAllOnesValue(lType);
  llvm::BinaryOperator* cmpl = llvm::BinaryOperator::create(
      llvm::Instruction::Xor,operand,allOnes,"cmpl",lblk);
  lops.push_back(cmpl);
}

template<> void
LLVMGeneratorPass::gen(PreIncrOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for PreIncrOp");
  llvm::Value* operand = lops.back(); lops.pop_back();
  const llvm::Type* lType = operand->getType();
  if (lType->isFloatingPoint()) {
    llvm::ConstantFP* one = llvm::ConstantFP::get(lType,1.0);
    llvm::BinaryOperator* add = llvm::BinaryOperator::create(
      llvm::Instruction::Add, operand, one, "preincr", lblk);
    lops.push_back(add);
  } else if (lType->isInteger()) {
    llvm::ConstantInt* one = llvm::ConstantInt::get(lType,1);
    llvm::BinaryOperator* add = llvm::BinaryOperator::create(
      llvm::Instruction::Add, operand, one, "preincr", lblk);
    lops.push_back(add);
  } else {
    hlvmAssert(!"PreIncrOp on non-numeric");
  }
}

template<> void
LLVMGeneratorPass::gen(PreDecrOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for PreDecrOp");
  llvm::Value* operand = lops.back(); lops.pop_back();
  const llvm::Type* lType = operand->getType();
  if (lType->isFloatingPoint()) {
    llvm::ConstantFP* one = llvm::ConstantFP::get(lType,1.0);
    llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
      llvm::Instruction::Sub, operand, one, "predecr", lblk);
    lops.push_back(sub);
  } else if (lType->isInteger()) {
    llvm::ConstantInt* one = llvm::ConstantInt::get(lType,1);
    llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
      llvm::Instruction::Sub, operand, one, "predecr", lblk);
    lops.push_back(sub);
  } else {
    hlvmAssert(!"PreIncrOp on non-numeric");
  }
}

template<> void
LLVMGeneratorPass::gen(PostIncrOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for PostIncrOp");
}

template<> void
LLVMGeneratorPass::gen(PostDecrOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for PostDecrOp");
}

template<> void
LLVMGeneratorPass::gen(AddOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for AddOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* add = llvm::BinaryOperator::create(
    llvm::Instruction::Add, op1, op2, "add", lblk);
  lops.push_back(add);
}

template<> void
LLVMGeneratorPass::gen(SubtractOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for SubtractOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* sub = llvm::BinaryOperator::create(
    llvm::Instruction::Sub, op1, op2, "add", lblk);
  lops.push_back(sub);
}

template<> void
LLVMGeneratorPass::gen(MultiplyOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for MultiplyOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* mul = llvm::BinaryOperator::create(
    llvm::Instruction::Mul, op1, op2, "mul", lblk);
  lops.push_back(mul);
}

template<> void
LLVMGeneratorPass::gen(DivideOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for DivideOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* div = llvm::BinaryOperator::create(
    llvm::Instruction::Div, op1, op2, "div", lblk);
  lops.push_back(div);
}

template<> void
LLVMGeneratorPass::gen(ModuloOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for ModuloOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* rem = llvm::BinaryOperator::create(
    llvm::Instruction::Rem, op1, op2, "mod", lblk);
  lops.push_back(rem);
}

template<> void
LLVMGeneratorPass::gen(BAndOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for BAndOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* band = llvm::BinaryOperator::create(
    llvm::Instruction::And, op1, op2, "band", lblk);
  lops.push_back(band);
}

template<> void
LLVMGeneratorPass::gen(BOrOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for BOrOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* bor = llvm::BinaryOperator::create(
    llvm::Instruction::Or, op1, op2, "bor", lblk);
  lops.push_back(bor);
}

template<> void
LLVMGeneratorPass::gen(BXorOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for BXorOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* Xor = llvm::BinaryOperator::create(
    llvm::Instruction::Xor, op1, op2, "bxor", lblk);
  lops.push_back(Xor);
}

template<> void
LLVMGeneratorPass::gen(BNorOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for BNorOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::BinaryOperator* bor = llvm::BinaryOperator::create(
    llvm::Instruction::Or, op1, op2, "bnor", lblk);
  llvm::BinaryOperator* nor = llvm::BinaryOperator::createNot(bor,"bnor",lblk);
  lops.push_back(nor);
}

template<> void
LLVMGeneratorPass::gen(NullOp* op)
{
  // Not surprisingly, there's nothing to do here.
}

template<> void
LLVMGeneratorPass::gen(NotOp* op)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for BNorOp");
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  llvm::Value* b1 = toBoolean(op1);
  llvm::BinaryOperator* Not = llvm::BinaryOperator::createNot(b1,"not",lblk);
  lops.push_back(Not);
}

template<> void
LLVMGeneratorPass::gen(AndOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for BNorOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* And = llvm::BinaryOperator::create(
    llvm::Instruction::And, b1, b2, "and", lblk);
  lops.push_back(And);
}

template<> void
LLVMGeneratorPass::gen(OrOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for BNorOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* Or = llvm::BinaryOperator::create(
    llvm::Instruction::Or, b1, b2, "or", lblk);
  lops.push_back(Or);
}

template<> void
LLVMGeneratorPass::gen(NorOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for NorOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* Or = llvm::BinaryOperator::create(
    llvm::Instruction::Or, b1, b2, "nor", lblk);
  llvm::BinaryOperator* Nor = llvm::BinaryOperator::createNot(Or,"nor",lblk);
  lops.push_back(Nor);
}

template<> void
LLVMGeneratorPass::gen(XorOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for XorOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::Value* b1 = toBoolean(op1);
  llvm::Value* b2 = toBoolean(op2);
  llvm::BinaryOperator* Xor = llvm::BinaryOperator::create(
    llvm::Instruction::Xor, b1, b2, "xor", lblk);
  lops.push_back(Xor);
}

template<> void
LLVMGeneratorPass::gen(EqualityOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for EqualityOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::SetCondInst* SCI = 
    new llvm::SetCondInst(llvm::Instruction::SetEQ, op1,op2,"eq",lblk);
  lops.push_back(SCI);
}

template<> void
LLVMGeneratorPass::gen(InequalityOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for InequalityOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  llvm::SetCondInst* SCI = 
    new llvm::SetCondInst(llvm::Instruction::SetNE, op1,op2,"ne",lblk);
  lops.push_back(SCI);
}

template<> void
LLVMGeneratorPass::gen(LessThanOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for LessThanOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  llvm::SetCondInst* SCI = 
    new llvm::SetCondInst(llvm::Instruction::SetLT, op1,op2,"lt",lblk);
  lops.push_back(SCI);
}

template<> void
LLVMGeneratorPass::gen(GreaterThanOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for GreaterThanOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  llvm::SetCondInst* SCI = 
    new llvm::SetCondInst(llvm::Instruction::SetGT, op1,op2,"gt",lblk);
  lops.push_back(SCI);
}

template<> void
LLVMGeneratorPass::gen(GreaterEqualOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for GreaterEqualOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  llvm::SetCondInst* SCI = 
    new llvm::SetCondInst(llvm::Instruction::SetGE, op1,op2,"ge",lblk);
  lops.push_back(SCI);
}

template<> void
LLVMGeneratorPass::gen(LessEqualOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for LessEqualOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  llvm::SetCondInst* SCI = 
    new llvm::SetCondInst(llvm::Instruction::SetLE, op1,op2,"le",lblk);
  lops.push_back(SCI);
}

template<> void
LLVMGeneratorPass::gen(SelectOp* op)
{
  hlvmAssert(lops.size() >= 3 && "Too few operands for SelectOp");
  llvm::Value* op3 = lops.back(); lops.pop_back();
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
  hlvmAssert(op1->getType() == llvm::Type::BoolTy);
  hlvmAssert(op2->getType() == op2->getType());
  hlvmAssert(llvm::isa<llvm::BasicBlock>(op2) == 
             llvm::isa<llvm::BasicBlock>(op3));
  if (llvm::isa<llvm::BasicBlock>(op2)) {
    // both are blocks, emit a BranchInstr
    lops.push_back(new llvm::BranchInst(
      llvm::cast<llvm::BasicBlock>(op2),
      llvm::cast<llvm::BasicBlock>(op3),op1,lblk));
    return;
  } 

  // A this point, we can only be left with a first class type since all HLVM
  // operators translate to a first class type. Since the select operator
  // requires first class types, its okay to just use it here.
  hlvmAssert(op2->getType()->isFirstClassType());
  lops.push_back(new llvm::SelectInst(op1,op2,op3,"select",lblk));
}

template<> void
LLVMGeneratorPass::gen(SwitchOp* op)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for SwitchOp");
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();

}

template<> void
LLVMGeneratorPass::gen(LoopOp* op)
{
  hlvmAssert(lops.size() >= 3 && "Too few operands for SelectOp");
  llvm::Value* op3 = lops.back(); lops.pop_back();
  llvm::Value* op2 = lops.back(); lops.pop_back();
  llvm::Value* op1 = lops.back(); lops.pop_back();
}

template<> void
LLVMGeneratorPass::gen(BreakOp* op)
{
}

template<> void
LLVMGeneratorPass::gen(ContinueOp* op)
{
}

template<> void
LLVMGeneratorPass::gen(ReturnOp* r)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for ReturnInst");
  llvm::Value* retVal = lops.back(); lops.pop_back();
  const llvm::Type* retTy = retVal->getType();
  if (retTy != lfunc->getReturnType()) {
    retVal = new llvm::CastInst(retVal,lfunc->getReturnType(),"",lblk);
  }
  // RetInst is never the operand of another instruction because it is
  // a terminator and cannot return a value. Consequently, we don't push it
  // on the lops stack.
  new llvm::ReturnInst(retVal,lblk);
}

template<> void
LLVMGeneratorPass::gen(CallOp* co)
{
  hlvm::Function* hFunc = co->getCalledFunction();
  const SignatureType* sigTy = hFunc->getSignature();
  std::vector<llvm::Value*> args;
  hlvmAssert(lops.size() >= sigTy->size()+1 && "Too few operands for CallOp");
  if (sigTy->size() > 0) {
    for (unsigned i = sigTy->size()-1; i >= 0; i++)
      args.push_back(lops.back()); lops.pop_back();
  }
  hlvmAssert(llvm::isa<llvm::Function>(lops.back()));
  llvm::Function* F = llvm::cast<llvm::Function>(lops.back()); lops.pop_back();
  lops.push_back(new llvm::CallInst(F,args,"call_" + hFunc->getName(),lblk));
}

template<> void
LLVMGeneratorPass::gen(StoreOp* s)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for StoreOp");
  llvm::Value* value =    lops.back(); lops.pop_back();
  llvm::Value* location = lops.back(); lops.pop_back();
  lops.push_back(new llvm::StoreInst(value,location,lblk));
}

template<> void
LLVMGeneratorPass::gen(LoadOp* s)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for LoadOp");
  llvm::Value* location = lops.back(); lops.pop_back();
  lops.push_back(new llvm::LoadInst(location,"",lblk));
}

template<> void
LLVMGeneratorPass::gen(ReferenceOp* r)
{
  hlvm::Value* referent = const_cast<hlvm::Value*>(r->getReferent());
  llvm::Value* v = 0;
  if (llvm::isa<AutoVarOp>(referent)) 
  {
    AutoVarDictionary::const_iterator I = 
      lvars.find(llvm::cast<AutoVarOp>(referent));
    hlvmAssert(I != lvars.end());
    v = I->second;
  }
  else if (llvm::isa<ConstantValue>(referent))
  {
    const hlvm::ConstantValue* cval = llvm::cast<ConstantValue>(referent);
    llvm::Constant* C = getConstant(cval);
    hlvmAssert(C && "Can't generate constant?");
    v = C;
  }
  else if (llvm::isa<Variable>(referent)) 
  {
    llvm::Value* V = getVariable(llvm::cast<hlvm::Variable>(referent));
    hlvmAssert(V && "Variable not found?");
    v = V;
  } 
  else if (llvm::isa<Function>(referent)) 
  {
    llvm::Function* F = getFunction(llvm::cast<hlvm::Function>(referent));
    hlvmAssert(F && "Function not found?");
    v = F;
  }
  else
    hlvmDeadCode("Referent not a linkable or autovar?");
  lops.push_back(v);
}

template<> void
LLVMGeneratorPass::gen(IndexOp* r)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for IndexOp");
}

template<> void
LLVMGeneratorPass::gen(OpenOp* o)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for OpenOp");
  llvm::Value* strm = lops.back(); lops.pop_back();
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
    llvm::CallInst(get_hlvm_stream_open(), args, "", lblk);
  lops.push_back(ci);
}

template<> void
LLVMGeneratorPass::gen(WriteOp* o)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for WriteOp");
  llvm::Value* arg2 = lops.back(); lops.pop_back();
  llvm::Value* strm = lops.back(); lops.pop_back();
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  args.push_back(arg2);
  if (llvm::isa<llvm::PointerType>(arg2->getType()))
    if (llvm::cast<llvm::PointerType>(arg2->getType())->getElementType() ==
        llvm::Type::SByteTy)
      lops.push_back(
        new llvm::CallInst(get_hlvm_stream_write_string(), args, "", lblk));
  if (arg2->getType() == get_hlvm_text())
    lops.push_back(
      new llvm::CallInst(get_hlvm_stream_write_text(), args, "", lblk));
  else if (arg2->getType() == get_hlvm_buffer())
    lops.push_back(
      new llvm::CallInst(get_hlvm_stream_write_buffer(), args, "",lblk));
}

template<> void
LLVMGeneratorPass::gen(ReadOp* o)
{
  hlvmAssert(lops.size() >= 3 && "Too few operands for ReadOp");
  std::vector<llvm::Value*> args;
  args.insert(args.end(),lops.end()-3,lops.end());
  lops.erase(lops.end()-3,lops.end());
  llvm::CallInst* ci = 
    new llvm::CallInst(get_hlvm_stream_read(), args, "", lblk);
  lops.push_back(ci);
}

template<> void
LLVMGeneratorPass::gen(CloseOp* o)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for CloseOp");
  std::vector<llvm::Value*> args;
  args.push_back(lops.back()); lops.pop_back();
  llvm::CallInst* ci = 
    new llvm::CallInst(get_hlvm_stream_close(), args, "", lblk);
  lops.push_back(ci);
}

template<> void
LLVMGeneratorPass::gen(hlvm::Function* f)
{
  // Get/Create the function
  lfunc = getFunction(f);
  // Clear the LLVM vars
  lvars.clear();
}

template<> void
LLVMGeneratorPass::gen(Program* p)
{
  // Create a new function for the program based on the signature
  lfunc = new llvm::Function(
      /*Type=*/ get_hlvm_program_signature(),
      /*Linkage=*/llvm::GlobalValue::InternalLinkage,
      /*Name=*/ getLinkageName(p),
      /*Module=*/ lmod
  );
  // Clear LLVM vars
  lvars.clear();

  // Save the program so it can be generated into the list of program entry
  progs.push_back(lfunc);
}

template<> void
LLVMGeneratorPass::gen(Bundle* b)
{
  lmod = new llvm::Module(b->getName());
  modules.push_back(lmod);
  lvars.clear();
  gvars.clear();
  funcs.clear();
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
    switch (n->getID()) {
    case BundleID:                gen(llvm::cast<Bundle>(n)); break;
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
    case FunctionID:              gen(llvm::cast<hlvm::Function>(n)); break;
    case ProgramID:               gen(llvm::cast<Program>(n)); break;
    case BlockID:                 gen(llvm::cast<Block>(n)); break;
    default:
      break;
    }
  } else {
    // We process non-container nodes and operators. Operators are done
    // post-order because we want their operands to be constructed first.
    switch (n->getID()) 
    {
    case AliasTypeID:             gen(llvm::cast<AliasType>(n)); break;
    case AnyTypeID:               gen(llvm::cast<AnyType>(n)); break;
    case StringTypeID:            gen(llvm::cast<StringType>(n)); break;
    case BooleanTypeID:           gen(llvm::cast<BooleanType>(n)); break;
    case CharacterTypeID:         gen(llvm::cast<CharacterType>(n)); break;
    case IntegerTypeID:           gen(llvm::cast<IntegerType>(n)); break;
    case RangeTypeID:             gen(llvm::cast<RangeType>(n)); break;
    case EnumerationTypeID:       gen(llvm::cast<EnumerationType>(n)); break;
    case RealTypeID:              gen(llvm::cast<RealType>(n)); break;
    case OctetTypeID:             gen(llvm::cast<OctetType>(n)); break;
    case VoidTypeID:              gen(llvm::cast<VoidType>(n)); break;
    case PointerTypeID:           gen(llvm::cast<hlvm::PointerType>(n)); break;
    case ArrayTypeID:             gen(llvm::cast<hlvm::ArrayType>(n)); break;
    case VectorTypeID:            gen(llvm::cast<VectorType>(n)); break;
    case StructureTypeID:         gen(llvm::cast<StructureType>(n)); break;
    case SignatureTypeID:         gen(llvm::cast<SignatureType>(n)); break;
    case OpaqueTypeID:            gen(llvm::cast<hlvm::OpaqueType>(n)); break;
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
    case NullOpID:                gen(llvm::cast<NullOp>(n)); break;
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
    case LoopOpID:                gen(llvm::cast<LoopOp>(n)); break;
    case BreakOpID:               gen(llvm::cast<BreakOp>(n)); break;
    case ContinueOpID:            gen(llvm::cast<ContinueOp>(n)); break;
    case ReturnOpID:              gen(llvm::cast<ReturnOp>(n)); break;
    case CallOpID:                gen(llvm::cast<CallOp>(n)); break;
    case LoadOpID:                gen(llvm::cast<LoadOp>(n)); break;
    case StoreOpID:               gen(llvm::cast<StoreOp>(n)); break;
    case ReferenceOpID:           gen(llvm::cast<ReferenceOp>(n)); break;
    case AutoVarOpID:             gen(llvm::cast<AutoVarOp>(n)); break;
    case OpenOpID:                gen(llvm::cast<OpenOp>(n)); break;
    case CloseOpID:               gen(llvm::cast<CloseOp>(n)); break;
    case WriteOpID:               gen(llvm::cast<WriteOp>(n)); break;
    case ReadOpID:                gen(llvm::cast<ReadOp>(n)); break;

    // ignore end of block, program, function and bundle
    case BundleID:
      genProgramLinkage();
      lmod = 0;
      break;
    case ConstantBooleanID:       
    case ConstantIntegerID:      
    case ConstantRealID:        
    case ConstantStringID:     
    case VariableID: 
      break;
    case ProgramID:
    case FunctionID:
      lfunc = 0;
      break;
    case BlockID:
      lblk = 0;
      break;

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

}

void
hlvm::generateBytecode(AST* tree, std::ostream& output, bool verify)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  llvm::Module* mod = genPass.linkModules();
  llvm::verifyModule(*mod, llvm::PrintMessageAction);
  llvm::WriteBytecodeToFile(mod, output, /*compress= */ true);
  delete mod;
  delete PM;
}

void
hlvm::generateAssembly(AST* tree, std::ostream& output, bool verify)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  llvm::Module* mod = genPass.linkModules();
  llvm::PassManager Passes;
  Passes.add(new llvm::PrintModulePass(&output));
  Passes.run(*mod);
  delete mod;
  delete PM;
}
