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
  public:
    LLVMGeneratorPass(const AST* tree)
      : Pass(0,Pass::PreAndPostOrderTraversal),
      modules(), lmod(0), lfunc(0), lblk(0), linst(), lops(), ltypes(),
      ast(tree),   bundle(0), function(0), block(0) { }
    ~LLVMGeneratorPass() { }

  /// Conversion functions
  inline const llvm::Type* getType(const hlvm::Type* ty);
  inline llvm::GlobalValue::LinkageTypes getLinkageTypes(LinkageKinds lk);
  inline std::string getLinkageName(LinkageItem* li);

  /// Generators
  inline void gen(Bundle* b);
  inline void gen(AliasType* t);
  inline void gen(AnyType* t);
  inline void gen(BooleanType* t);
  inline void gen(CharacterType* t);
  inline void gen(IntegerType* t);
  inline void gen(RangeType* t);
  inline void gen(EnumerationType* t);
  inline void gen(RealType* t);
  inline void gen(OctetType* t);
  inline void gen(VoidType* t);
  inline void gen(hlvm::PointerType* t);
  inline void gen(hlvm::ArrayType* t);
  inline void gen(VectorType* t);
  inline void gen(StructureType* t);
  inline void gen(SignatureType* t);
  inline void gen(ConstantInteger* t);
  inline void gen(ConstantText* t);
  inline void gen(Variable* v);
  inline void gen(hlvm::Function* f);
  inline void gen(Program* p);
  inline void gen(Block* f);
  inline void gen(ReturnOp* f);

  virtual void handleInitialize();
  virtual void handle(Node* n,Pass::TraversalKinds mode);
  virtual void handleTerminate();

  inline llvm::Module* linkModules();

private:
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
  std::vector<llvm::Function*> progs; ///< The list of programs to emit at the end
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
    case ArrayTypeID: {
      const llvm::Type* elemType = 
        getType(llvm::cast<hlvm::ArrayType>(ty)->getElementType());
      std::vector<const llvm::Type*> Fields;
      Fields.push_back(llvm::Type::UIntTy);
      Fields.push_back(llvm::PointerType::get(elemType));
      result = llvm::StructType::get(Fields);
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

void 
LLVMGeneratorPass::gen(AliasType* t)
{
}
void 
LLVMGeneratorPass::gen(AnyType* t)
{
}

void
LLVMGeneratorPass::gen(BooleanType* t)
{
}

void
LLVMGeneratorPass::gen(CharacterType* t)
{
}

void
LLVMGeneratorPass::gen(IntegerType* t)
{
}

void
LLVMGeneratorPass::gen(RangeType* t)
{
}

void 
LLVMGeneratorPass::gen(EnumerationType* t)
{
}

void
LLVMGeneratorPass::gen(RealType* t)
{
}

void
LLVMGeneratorPass::gen(OctetType* t)
{
}

void
LLVMGeneratorPass::gen(VoidType* t)
{
}

void 
LLVMGeneratorPass::gen(hlvm::PointerType* t)
{
}

void 
LLVMGeneratorPass::gen(hlvm::ArrayType* t)
{
}

void 
LLVMGeneratorPass::gen(VectorType* t)
{
}

void 
LLVMGeneratorPass::gen(StructureType* t)
{
}

void 
LLVMGeneratorPass::gen(SignatureType* t)
{
}

void 
LLVMGeneratorPass::gen(ConstantInteger* i)
{
  const hlvm::Type* hType = i->getType();
  const llvm::Type* lType = getType(hType);
  if (llvm::cast<IntegerType>(hType)->isSigned())
    lops.push_back(llvm::ConstantSInt::get(lType,i->getValue()));
  else
    lops.push_back(llvm::ConstantUInt::get(lType,i->getValue(0)));
}

void
LLVMGeneratorPass::gen(ConstantText* t)
{
}

void
LLVMGeneratorPass::gen(Variable* v)
{
}

void 
LLVMGeneratorPass::gen(Block* b)
{
  lblk = new llvm::BasicBlock(b->getLabel(),lfunc,0);
}

void
LLVMGeneratorPass::gen(ReturnOp* r)
{
  llvm::ReturnInst* ret = 0;
  if (lops.empty())
    ret =  new llvm::ReturnInst(lblk);
  else {
    hlvmAssert(lops.size() == 1 && "Too many operands for ReturnInst");
    llvm::Value* retVal = lops[0];;
    const llvm::Type* retTy = retVal->getType();
    if (retTy != lfunc->getReturnType()) {
      retVal = new llvm::CastInst(retVal,lfunc->getReturnType(),"",lblk);
    }
    ret = new llvm::ReturnInst(retVal,lblk);
    lops.clear();
  }
  // RetInst is never the operand of another instruction (Terminator)
}

llvm::GlobalValue::LinkageTypes
LLVMGeneratorPass::getLinkageTypes(LinkageKinds lk)
{
  return llvm::GlobalValue::LinkageTypes(lk);
}

std::string
LLVMGeneratorPass::getLinkageName(LinkageItem* lk)
{
  if (lk->isProgram())
    return std::string("_hlvm_entry_") + lk->getName();
  // FIXME: This needs to incorporate the bundle name
  return lk->getName();
}

void
LLVMGeneratorPass::gen(hlvm::Function* f)
{
  lfunc = new llvm::Function(
    llvm::cast<llvm::FunctionType>(getType(f->getSignature())),
    getLinkageTypes(f->getLinkageKind()), 
    getLinkageName(f), lmod);
}

void
LLVMGeneratorPass::gen(Program* p)
{
  // points after the entire parse is completed.
  std::string linkageName = getLinkageName(p);

  // Get the FunctionType for entry points (programs)
  const llvm::FunctionType* entry_signature = 
    llvm::cast<llvm::FunctionType>(getType(p->getSignature()));

  // Create a new function for the program based on the signature
  lfunc = new llvm::Function(entry_signature,
    llvm::GlobalValue::InternalLinkage,linkageName,lmod);

  // Save the program so it can be generated into the list of program entry
  progs.push_back(lfunc);
}

void
LLVMGeneratorPass::gen(Bundle* b)
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
    case ConstantIntegerID:       gen(llvm::cast<ConstantInteger>(n));break;
    case ConstantTextID:          gen(llvm::cast<ConstantText>(n));break;
    case VariableID:              gen(llvm::cast<Variable>(n)); break;
    case ReturnOpID:              gen(llvm::cast<ReturnOp>(n)); break;
    default:
      break;
    }
  }
}

void
LLVMGeneratorPass::handleTerminate()
{
  // Get the type of function that all entry points must have
  const llvm::FunctionType* entry_signature = 
    llvm::cast<llvm::FunctionType>(getType(ast->getProgramType())); 

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
    /*Name=*/"_hlvm_entry_points",
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
