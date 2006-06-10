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
/// @file hlvm/CodeGen/LLVM/LLVMGenerator.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the implementation of the HLVM -> LLVM Code Generator
//===----------------------------------------------------------------------===//

#include <hlvm/CodeGen/LLVMGenerator.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Variable.h>
#include <hlvm/AST/Program.h>
#include <hlvm/AST/Block.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
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

namespace {
using namespace llvm;
#include <hlvm/CodeGen/string_decl.inc>
#include <hlvm/CodeGen/string_clear.inc>
#include <hlvm/CodeGen/program.inc>
}

namespace 
{
using namespace hlvm;

class LLVMGeneratorPass : public hlvm::Pass
{
  typedef std::vector<llvm::Module*> ModuleList;
  typedef std::vector<llvm::Value*> OperandList;
  typedef std::map<hlvm::Variable*,llvm::Value*> VariableDictionary;
  typedef std::map<hlvm::AutoVarOp*,llvm::Value*> AutoVarDictionary;
  typedef std::map<hlvm::Constant*,llvm::Constant*> ConstantDictionary;
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
      hlvm_stream_close(0), hlvm_program_signature(0)
      { }
    ~LLVMGeneratorPass() { }

  /// Conversion functions
  const llvm::Type* getType(const hlvm::Type* ty);
  llvm::Constant* getConstant(const hlvm::Constant* C);
  llvm::Value* getVariable(const hlvm::Variable* V);
  inline llvm::GlobalValue::LinkageTypes getLinkageTypes(LinkageKinds lk);
  inline std::string getLinkageName(LinkageItem* li);

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
  inline llvm::Function*     get_hlvm_stream_close();
  inline llvm::FunctionType* get_hlvm_program_signature();

  /// Generator
  template <class NodeClass>
  inline void gen(NodeClass *nc);

  virtual void handleInitialize();
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
    arg_types.push_back(get_hlvm_text());
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
      hlvm::Type* hElemType = 
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
LLVMGeneratorPass::getConstant(const hlvm::Constant* C)
{
  if (C == 0)
    return 0;

  // First, lets see if its cached already
  ConstantDictionary::iterator I = consts.find(const_cast<hlvm::Constant*>(C));
  if (I != consts.end())
    return I->second;

  const hlvm::Type* hType = C->getType();
  const llvm::Type* lType = getType(hType);
  llvm::Constant* result = 0;
  switch (C->getID()) 
  {
    case ConstantIntegerID:
    {
      const ConstantInteger* CI = llvm::cast<const ConstantInteger>(C);
      if (llvm::cast<IntegerType>(hType)->isSigned())
        result = llvm::ConstantSInt::get(lType,CI->getValue());
      else
        result = llvm::ConstantUInt::get(lType,CI->getValue(0));
      break;
    }
    case ConstantRealID:
    {
      result = llvm::ConstantFP::get(lType,
        llvm::cast<ConstantReal>(C)->getValue());
      break;
    }
    case ConstantTextID:
    {
      llvm::Constant* CA = llvm::ConstantArray::get(
        llvm::cast<ConstantText>(C)->getValue(),true);
      llvm::GlobalVariable* GV = new llvm::GlobalVariable(
        CA->getType(), true, llvm::GlobalValue::InternalLinkage, CA, "", lmod);
      result = ConstantExpr::getPtrPtrFromArrayPtr(GV);
      break;
    }
    case ConstantZeroID:
    {
      result = llvm::Constant::getNullValue(lType);
      break;
    }
    default:
      break;
  }
  if (result)
    consts[const_cast<hlvm::Constant*>(C)] = result;
  else
    hlvmDeadCode("Didn't find constant");
  return result;
}

llvm::Value*
LLVMGeneratorPass::getVariable(const Variable* V) 
{
  // FIXME: implement
  return 0;
}


llvm::GlobalValue::LinkageTypes
LLVMGeneratorPass::getLinkageTypes(LinkageKinds lk)
{
  return llvm::GlobalValue::LinkageTypes(lk);
}

std::string
LLVMGeneratorPass::getLinkageName(LinkageItem* lk)
{
  // if (lk->isProgram())
    // return std::string("_hlvm_entry_") + lk->getName();
  // FIXME: This needs to incorporate the bundle name
  return lk->getName();
}

template<> void 
LLVMGeneratorPass::gen<AliasType>(AliasType* t)
{
  lmod->addTypeName(t->getName(), getType(t->getElementType()));
}

template<> void 
LLVMGeneratorPass::gen<AnyType>(AnyType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<BooleanType>(BooleanType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<CharacterType>(CharacterType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<IntegerType>(IntegerType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<RangeType>(RangeType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<EnumerationType>(EnumerationType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<RealType>(RealType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<OctetType>(OctetType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void
LLVMGeneratorPass::gen<VoidType>(VoidType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<hlvm::PointerType>(hlvm::PointerType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<hlvm::ArrayType>(hlvm::ArrayType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<VectorType>(VectorType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<StructureType>(StructureType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<SignatureType>(SignatureType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<hlvm::OpaqueType>(hlvm::OpaqueType* t)
{
  lmod->addTypeName(t->getName(), getType(t));
}

template<> void 
LLVMGeneratorPass::gen<ConstantInteger>(ConstantInteger* i)
{
  llvm::Constant* C = getConstant(i);
  lops.push_back(C);
}

template<> void
LLVMGeneratorPass::gen<ConstantReal>(ConstantReal* r)
{
  llvm::Constant* C = getConstant(r);
  lops.push_back(C);
}

template<> void
LLVMGeneratorPass::gen<ConstantZero>(ConstantZero* z)
{
  llvm::Constant* C = getConstant(z);
  lops.push_back(C);
}

template<> void
LLVMGeneratorPass::gen<ConstantText>(ConstantText* t)
{
  llvm::Constant* C = getConstant(t);
  std::vector<llvm::Value*> args;
  args.push_back(C);
  llvm::CallInst* CI = new CallInst(get_hlvm_text_create(),args,"",lblk);
  lops.push_back(CI);
}

template<> void
LLVMGeneratorPass::gen<AutoVarOp>(AutoVarOp* av)
{
  assert(lblk  != 0 && "Not in block context");
  // emit a stack variable
  const llvm::Type* elemType = getType(av->getType());
  llvm::Value* init = lops.back(); lops.pop_back();
  llvm::Value* alloca = new llvm::AllocaInst(
      /*Ty=*/ elemType,
      /*ArraySize=*/ llvm::ConstantUInt::get(llvm::Type::UIntTy,1),
      /*Name=*/ av->getName(),
      /*InsertAtEnd=*/ lblk
  );
  // FIXME: Handle initializer
  llvm::Constant* C = getConstant(av->getInitializer());
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
            // The initializer is an sbyte*, which we can conver to either a
            // hlvm_text or an hlvm_buffer.
            if (elemType = get_hlvm_buffer()) {
              // Assign the constant string to the buffer
            } else if (elemType == get_hlvm_text()) {
            }
          }
        }
      }
      hlvmAssert(CType->isLosslesslyConvertibleTo(elemType));
      C = ConstantExpr::getCast(C,elemType);
    }
    llvm::Value* store = new llvm::StoreInst(C,alloca,"",lblk);
  }
  lops.push_back(alloca);
  lvars[av] = alloca;
}

template<> void
LLVMGeneratorPass::gen<Variable>(Variable* v)
{
  llvm::Constant* Initializer = getConstant(v->getInitializer());
  llvm::Value* gv = new llvm::GlobalVariable(
    /*Ty=*/ getType(v->getType()),
    /*isConstant=*/ false,
    /*Linkage=*/ llvm::GlobalValue::ExternalLinkage,
    /*Initializer=*/ Initializer,
    /*Name=*/ getLinkageName(v),
    /*Parent=*/ lmod
  );
  gvars[v] = gv;
}

template<> void 
LLVMGeneratorPass::gen<Block>(Block* b)
{
  lblk = new llvm::BasicBlock(b->getLabel(),lfunc,0);
}

template<> void
LLVMGeneratorPass::gen<ReturnOp>(ReturnOp* r)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for ReturnInst");
  llvm::Value* retVal = lops.back(); lops.pop_back();
  const llvm::Type* retTy = retVal->getType();
  if (retTy != lfunc->getReturnType()) {
    retVal = new llvm::CastInst(retVal,lfunc->getReturnType(),"",lblk);
  }
  new llvm::ReturnInst(retVal,lblk);
  // RetInst is never the operand of another instruction (Terminator)
}

template<> void
LLVMGeneratorPass::gen<StoreOp>(StoreOp* s)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for StoreOp");
  llvm::Value* value =    lops.back(); lops.pop_back();
  llvm::Value* location = lops.back(); lops.pop_back();
  lops.push_back(new StoreInst(value,location,lblk));
}

template<> void
LLVMGeneratorPass::gen<LoadOp>(LoadOp* s)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for LoadOp");
  llvm::Value* location = lops.back(); lops.pop_back();
  lops.push_back(new LoadInst(location,"",lblk));
}

template<> void
LLVMGeneratorPass::gen<ReferenceOp>(ReferenceOp* r)
{
  hlvm::Value* referent = r->getReferent();
  llvm::Value* v = 0;
  if (isa<Variable>(referent)) {
    VariableDictionary::iterator I = gvars.find(cast<Variable>(referent));
    hlvmAssert(I != gvars.end());
    v = I->second;
  } 
  else if (isa<AutoVarOp>(referent)) 
  {
    AutoVarDictionary::const_iterator I = lvars.find(cast<AutoVarOp>(referent));
    hlvmAssert(I != lvars.end());
    v = I->second;
  }
  else
    hlvmDeadCode("Referent not a variable");
  lops.push_back(v);
}

template<> void
LLVMGeneratorPass::gen<IndexOp>(IndexOp* r)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for IndexOp");
}

template<> void
LLVMGeneratorPass::gen<OpenOp>(OpenOp* o)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for OpenOp");
  std::vector<llvm::Value*> args;
  args.push_back(lops.back()); lops.pop_back();
  llvm::CallInst* ci = new CallInst(get_hlvm_stream_open(), args, "", lblk);
  lops.push_back(ci);
}

template<> void
LLVMGeneratorPass::gen<WriteOp>(WriteOp* o)
{
  hlvmAssert(lops.size() >= 2 && "Too few operands for WriteOp");
  std::vector<llvm::Value*> args;
  llvm::Value* arg2 = lops.back(); lops.pop_back();
  llvm::Value* strm = lops.back(); lops.pop_back();
  args.push_back(strm);
  args.push_back(arg2);
  if (arg2->getType() == get_hlvm_text())
    lops.push_back(new CallInst(get_hlvm_stream_write_text(), args, "", lblk));
  else
    lops.push_back(new CallInst(get_hlvm_stream_write_buffer(), args, "",lblk));
}

template<> void
LLVMGeneratorPass::gen<ReadOp>(ReadOp* o)
{
  hlvmAssert(lops.size() >= 3 && "Too few operands for ReadOp");
  std::vector<llvm::Value*> args;
  args.insert(args.end(),lops.end()-3,lops.end());
  lops.erase(lops.end()-3,lops.end());
  llvm::CallInst* ci = new CallInst(get_hlvm_stream_read(), args, "", lblk);
  lops.push_back(ci);
}

template<> void
LLVMGeneratorPass::gen<CloseOp>(CloseOp* o)
{
  hlvmAssert(lops.size() >= 1 && "Too few operands for CloseOp");
  std::vector<llvm::Value*> args;
  args.push_back(lops.back()); lops.pop_back();
  llvm::CallInst* ci = new CallInst(get_hlvm_stream_close(), args, "", lblk);
  lops.push_back(ci);
}

template<> void
LLVMGeneratorPass::gen<hlvm::Function>(hlvm::Function* f)
{
  lfunc = new llvm::Function(
    llvm::cast<llvm::FunctionType>(getType(f->getSignature())),
    getLinkageTypes(f->getLinkageKind()), 
    getLinkageName(f), lmod);
  lvars.clear();
}

template<> void
LLVMGeneratorPass::gen<hlvm::Program>(Program* p)
{
  // points after the entire parse is completed.
  std::string linkageName = getLinkageName(p);

  // Create a new function for the program based on the signature
  lfunc = new llvm::Function(get_hlvm_program_signature(),
    llvm::GlobalValue::InternalLinkage,linkageName,lmod);
  lvars.clear();

  // Save the program so it can be generated into the list of program entry
  progs.push_back(lfunc);
}

template<> void
LLVMGeneratorPass::gen<Bundle>(Bundle* b)
{
  lmod = new llvm::Module(b->getName());
  modules.push_back(lmod);
  lvars.clear();
  gvars.clear();
}

void
LLVMGeneratorPass::handleInitialize()
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
    case ConstantZeroID:          gen(llvm::cast<ConstantZero>(n));break;
    case ConstantIntegerID:       gen(llvm::cast<ConstantInteger>(n));break;
    case ConstantRealID:          gen(llvm::cast<ConstantReal>(n));break;
    case ConstantTextID:          gen(llvm::cast<ConstantText>(n));break;
    case VariableID:              gen(llvm::cast<Variable>(n)); break;
    case ReturnOpID:              gen(llvm::cast<ReturnOp>(n)); break;
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
