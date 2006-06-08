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
#include <llvm/Constants.h>
#include <llvm/CallingConv.h>
#include <llvm/Linker.h>
#include <llvm/Bytecode/Writer.h>
#include <llvm/PassManager.h>
#include <llvm/Assembly/PrintModulePass.h>

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
  ModuleList modules;        ///< The list of modules we construct
  llvm::Module*     lmod;    ///< The current module we're generation 
  llvm::Function*   lfunc;   ///< The current LLVM function we're generating 
  llvm::BasicBlock* lblk;    ///< The current LLVM block we're generating
  llvm::BasicBlock::InstListType linst; 
  OperandList lops;          ///< The current list of instruction operands
  llvm::TypeSymbolTable ltypes; ///< The cached LLVM types we've generated
    ///< The current LLVM instructions we're generating
  const AST* ast;            ///< The current Tree we're traversing
  const Bundle* bundle;      ///< The current Bundle we're traversing
  const hlvm::Function* function;  ///< The current Function we're traversing
  const Block* block;        ///< The current Block we're traversing
  std::vector<llvm::Function*> progs; ///< The list of programs to emit
  const llvm::FunctionType* entry_FT; ///< The llvm function type for programs

  public:
    LLVMGeneratorPass(const AST* tree)
      : Pass(0,Pass::PreAndPostOrderTraversal),
      modules(), lmod(0), lfunc(0), lblk(0), linst(), lops(), ltypes(),
      ast(tree),   bundle(0), function(0), block(0), entry_FT(0) { }
    ~LLVMGeneratorPass() { }

  /// Conversion functions
  const llvm::Type* getType(const hlvm::Type* ty);
  llvm::Constant* getConstant(const hlvm::Constant* C);
  llvm::Value* getVariable(const hlvm::Variable* V);
  const llvm::FunctionType* getProgramFT();
  inline llvm::GlobalValue::LinkageTypes getLinkageTypes(LinkageKinds lk);
  inline std::string getLinkageName(LinkageItem* li);

  /// Generator
  template <class NodeClass>
  inline void gen(NodeClass *nc);

  virtual void handleInitialize();
  virtual void handle(Node* n,Pass::TraversalKinds mode);
  virtual void handleTerminate();

  inline llvm::Module* linkModules();
};

const llvm::Type*
LLVMGeneratorPass::getType(const hlvm::Type* ty)
{
  // First, lets see if its cached already
  const llvm::Type* result = ltypes.lookup(ty->getName());
  if (result)
    return result;

  // Okay, we haven't seen this type before so let's construct it
  switch (ty->getID()) {
    case VoidTypeID: result = llvm::Type::VoidTy; break;
    case BooleanTypeID: result = llvm::Type::BoolTy; break;
    case CharacterTypeID: result = llvm::Type::UShortTy; break;
    case OctetTypeID: result = llvm::Type::UByteTy; break;
    case UInt8TypeID: result = llvm::Type::UByteTy; break;
    case UInt16TypeID: result = llvm::Type::UShortTy; break;
    case UInt32TypeID: result = llvm::Type::UIntTy; break;
    case UInt64TypeID: result = llvm::Type::ULongTy; break;
    case UInt128TypeID: 
      hlvmNotImplemented("128 bit primitive integer");
      break;
    case SInt8TypeID: result = llvm::Type::SByteTy; break;
    case SInt16TypeID: result = llvm::Type::ShortTy; break;
    case SInt32TypeID: result = llvm::Type::IntTy; break;
    case SInt64TypeID: result = llvm::Type::LongTy; break;
    case SInt128TypeID: 
      hlvmNotImplemented("128 bit primitive integer");
      break;
    case Float32TypeID: result = llvm::Type::FloatTy; break;
    case Float64TypeID: result = llvm::Type::FloatTy; break;
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
    case TextTypeID: {
      std::vector<const llvm::Type*> Fields;
      Fields.push_back(llvm::Type::UIntTy);
      Fields.push_back(llvm::PointerType::get(llvm::Type::UShortTy));
      result = llvm::StructType::get(Fields);
      break;
    }
    case AliasTypeID: {
      result = getType(llvm::cast<AliasType>(ty)->getType());
      break;
    }
    case PointerTypeID: {
      result = llvm::PointerType::get(
        getType(llvm::cast<hlvm::PointerType>(ty)->getTargetType()));
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
  const hlvm::Type* hType = C->getType();
  const llvm::Type* lType = getType(hType);
  switch (C->getID()) 
  {
    case ConstantIntegerID:
    {
      const ConstantInteger* CI = llvm::cast<const ConstantInteger>(C);
      if (llvm::cast<IntegerType>(hType)->isSigned())
        return llvm::ConstantSInt::get(lType,CI->getValue());
      else
         return llvm::ConstantUInt::get(lType,CI->getValue(0));
    }
    case ConstantRealID:
      return llvm::ConstantFP::get(lType,
        llvm::cast<ConstantReal>(C)->getValue());
    case ConstantTextID:
      return llvm::ConstantArray::get(
        llvm::cast<ConstantText>(C)->getValue(),true);
    case ConstantZeroID:
    {
      switch (lType->getTypeID()) {
        case llvm::Type::VoidTyID:
          hlvmDeadCode("Can't get constant for void type");
          break;
        case llvm::Type::BoolTyID:
          return llvm::ConstantBool::get(false);
          break;
        case llvm::Type::UByteTyID:
        case llvm::Type::SByteTyID:
        case llvm::Type::UShortTyID:
        case llvm::Type::ShortTyID:
        case llvm::Type::UIntTyID:
        case llvm::Type::IntTyID:
        case llvm::Type::ULongTyID:
        case llvm::Type::LongTyID:
          return llvm::ConstantInt::get(lType,0);
          break;
        case llvm::Type::FloatTyID:
        case llvm::Type::DoubleTyID:
          return llvm::ConstantFP::get(lType,0.0);
          break;
        case llvm::Type::LabelTyID:
          hlvmDeadCode("Can't get constant for label type");
          break;
        case llvm::Type::FunctionTyID:
        case llvm::Type::StructTyID:
        case llvm::Type::ArrayTyID:
        case llvm::Type::PointerTyID:
        case llvm::Type::OpaqueTyID:
        case llvm::Type::PackedTyID:
          return llvm::ConstantAggregateZero::get(lType);
        default:
          hlvmDeadCode("Invalid LLVM Type ID");
          break;
      }
      break;
    }
    default:
      break;
  }
  hlvmDeadCode("Can't decipher constant");
  return 0;
}

llvm::Value*
LLVMGeneratorPass::getVariable(const Variable* V) 
{
  // FIXME: implement
  return 0;
}

const llvm::FunctionType*
LLVMGeneratorPass::getProgramFT()
{
  if (!entry_FT) {
    // Get the type of function that all entry points must have
    std::vector<const llvm::Type*> arg_types;
    arg_types.push_back(llvm::Type::IntTy);
    arg_types.push_back(
      llvm::PointerType::get(llvm::PointerType::get(llvm::Type::SByteTy)));
    entry_FT = llvm::FunctionType::get(llvm::Type::IntTy,arg_types,false);
    lmod->addTypeName("hlvm_program_signature",entry_FT);
  }
  return entry_FT;
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
  lmod->addTypeName(t->getName(), getType(t->getType()));
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
LLVMGeneratorPass::gen<ConstantText>(ConstantText* t)
{
  llvm::Constant* C = getConstant(t);
  lops.push_back(C);
}

template<> void
LLVMGeneratorPass::gen<Variable>(Variable* v)
{
  if (v->isLocal()) {
    assert(lblk  != 0 && "Not in block context");
    // emit a stack variable
    const llvm::Type* elemType = getType(v->getType());
    lops.push_back(
      new llvm::AllocaInst(
        /*Ty=*/ elemType,
        /*ArraySize=*/ llvm::ConstantUInt::get(llvm::Type::UIntTy,1),
        /*Name=*/ v->getName(),
        /*InsertAtEnd=*/ lblk
      )
    );
    // FIXME: Handle initializer
  } else {
    llvm::Constant* Initializer = getConstant(v->getInitializer());
    new llvm::GlobalVariable(
      /*Ty=*/ getType(v->getType()),
      /*isConstant=*/ false,
      /*Linkage=*/ llvm::GlobalValue::ExternalLinkage,
      /*Initializer=*/ Initializer,
      /*Name=*/ getLinkageName(v),
      /*Parent=*/ lmod);
  }
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
  llvm::Value* value = lops.back(); lops.pop_back();
  llvm::Value* location = lops.back(); lops.pop_back();
  lops.push_back(new StoreInst(location,value,lblk));
}

template<> void
LLVMGeneratorPass::gen<LoadOp>(LoadOp* s)
{
  hlvmAssert(lops.size() >= 1 && "Wrong number of ops for LoadOp");
  llvm::Value* location = lops.back(); lops.pop_back();
  lops.push_back(new LoadInst(location,"",lblk));
}

template<> void
LLVMGeneratorPass::gen<ReferenceOp>(ReferenceOp* r)
{
  hlvmAssert(lops.size() >= 0 && "Wrong number of ops for ReferenceOp");
  llvm::Value* v = getVariable(r->getReferent());
  lops.push_back(v);
}

template<> void
LLVMGeneratorPass::gen<IndexOp>(IndexOp* r)
{
  hlvmAssert(lops.size() >= 1 && "Wrong number of ops for IndexOp");
}

template<> void
LLVMGeneratorPass::gen<OpenOp>(OpenOp* o)
{
  hlvmAssert(lops.size() >= 1 && "Wrong number of ops for StoreOp");
  llvm::Value* uri = lops.back(); lops.pop_back();
}

template<> void
LLVMGeneratorPass::gen<WriteOp>(WriteOp* o)
{
  hlvmAssert(lops.size() >= 3 && "Wrong number of ops for StoreOp");
  llvm::Value* length = lops.back(); lops.pop_back();
  llvm::Value* buffer = lops.back(); lops.pop_back();
  llvm::Value* stream = lops.back(); lops.pop_back();
}

template<> void
LLVMGeneratorPass::gen<CloseOp>(CloseOp* o)
{
  hlvmAssert(lops.size() >= 1 && "Wrong number of ops for StoreOp");
  llvm::Value* stream = lops.back(); lops.pop_back();
}


template<> void
LLVMGeneratorPass::gen<hlvm::Function>(hlvm::Function* f)
{
  lfunc = new llvm::Function(
    llvm::cast<llvm::FunctionType>(getType(f->getSignature())),
    getLinkageTypes(f->getLinkageKind()), 
    getLinkageName(f), lmod);
}

template<> void
LLVMGeneratorPass::gen<hlvm::Program>(Program* p)
{
  // points after the entire parse is completed.
  std::string linkageName = getLinkageName(p);

  // Get the FunctionType for entry points (programs)
  const llvm::FunctionType* entry_signature = getProgramFT();

  // Create a new function for the program based on the signature
  lfunc = new llvm::Function(entry_signature,
    llvm::GlobalValue::InternalLinkage,linkageName,lmod);

  // Save the program so it can be generated into the list of program entry
  progs.push_back(lfunc);
}

template<> void
LLVMGeneratorPass::gen<Bundle>(Bundle* b)
{
  lmod = new llvm::Module(b->getName());
  modules.push_back(lmod);
}

void
LLVMGeneratorPass::handleInitialize()
{
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
    case ConstantIntegerID:       gen(llvm::cast<ConstantInteger>(n));break;
    case ConstantTextID:          gen(llvm::cast<ConstantText>(n));break;
    case VariableID:              gen(llvm::cast<Variable>(n)); break;
    case ReturnOpID:              gen(llvm::cast<ReturnOp>(n)); break;
    case LoadOpID:                gen(llvm::cast<LoadOp>(n)); break;
    case StoreOpID:               gen(llvm::cast<StoreOp>(n)); break;
    case ReferenceOpID:           gen(llvm::cast<ReferenceOp>(n)); break;
    case OpenOpID:                gen(llvm::cast<OpenOp>(n)); break;
    case CloseOpID:               gen(llvm::cast<CloseOp>(n)); break;
    case WriteOpID:               gen(llvm::cast<WriteOp>(n)); break;

    // ignore end of block, program, function and bundle
    case BundleID:
    case ProgramID:
    case FunctionID:
    case BlockID:
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
  // Get the type of function that all entry points must have
  const llvm::FunctionType* entry_signature = getProgramFT();

  // Define the type of the array elements (a structure with a pointer to
  // a string and a pointer to the function).
  std::vector<const llvm::Type*> Fields;
  Fields.push_back(llvm::PointerType::get(llvm::Type::SByteTy));
  Fields.push_back(llvm::PointerType::get(entry_signature));
  llvm::StructType* entry_elem_type = llvm::StructType::get(Fields);

  // Define the type of the array for the entry points
  llvm::ArrayType* entry_points_type = llvm::ArrayType::get(entry_elem_type,1);

  // Create a vector to hold the entry elements as they are created.
  std::vector<llvm::Constant*> entry_points_items;

  for (std::vector<llvm::Function*>::iterator I = progs.begin(), 
       E = progs.end(); I != E; ++I )
  {
    const std::string& funcName = (*I)->getName();
    // Get a constant for the name of the entry point (char array)
    llvm::Constant* name_val = llvm::ConstantArray::get(funcName,false);

    // Create a constant global variable to hold the name of the program.
    llvm::GlobalVariable* name = new llvm::GlobalVariable(
      /*Type=*/llvm::ArrayType::get(llvm::Type::SByteTy, funcName.length()),
      /*isConst=*/true,
      /*Linkage=*/llvm::GlobalValue::InternalLinkage, 
      /*Initializer=*/name_val, 
      /*name=*/"", 
      /*InsertInto=*/lmod
    );

    // Get the GEP for indexing in to the name string
    std::vector<llvm::Constant*> Indices;
    Indices.push_back(llvm::Constant::getNullValue(llvm::Type::IntTy));
    Indices.push_back(llvm::Constant::getNullValue(llvm::Type::IntTy));
    llvm::Constant* index = llvm::ConstantExpr::getGetElementPtr(name,Indices);

    // Get a constant structure for the entry containing the name and pointer
    // to the function.
    std::vector<llvm::Constant*> items;
    items.push_back(index);
    items.push_back(*I);
    llvm::Constant* entry = llvm::ConstantStruct::get(entry_elem_type,items);

    // Save the entry into the list of entry_point items
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
hlvm::generateBytecode(AST* tree,std::ostream& output)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  llvm::Module* mod = genPass.linkModules();
  llvm::WriteBytecodeToFile(mod, output, /*compress= */ true);
  delete mod;
  delete PM;
}

void
hlvm::generateAssembly(AST* tree, std::ostream& output)
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
