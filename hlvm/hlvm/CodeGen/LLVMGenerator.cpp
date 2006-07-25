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
#include <hlvm/CodeGen/LLVMEmitter.h>
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
  LLVMEmitter em;
  typedef std::map<const hlvm::Operator*,llvm::Value*> OperandMap;
  typedef std::map<const hlvm::Block*,llvm::BasicBlock*> BlockMap;
  typedef std::map<const hlvm::Operator*,llvm::BasicBlock*> LoopMap;
  typedef std::map<const hlvm::Block*,llvm::Instruction*> ResultsMap;
  typedef std::map<const hlvm::Variable*,llvm::Value*> VariableMap;
  typedef std::map<const hlvm::AutoVarOp*,llvm::Value*> AutoVarMap;
  typedef std::map<const hlvm::Constant*,llvm::Constant*> ConstantMap;
  typedef std::map<const hlvm::Function*,llvm::Function*> FunctionMap;
  ModuleList modules;           ///< The list of modules we construct
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

  public:
    LLVMGeneratorPass(const AST* tree)
      : Pass(0,Pass::PreAndPostOrderTraversal), em(),
      modules(), operands(), blocks(), breaks(), 
      continues(),
      gvars(), lvars(), ltypes(), consts(), funcs(),
      ast(tree), bundle(0), function(0), block(0)
      { }
    ~LLVMGeneratorPass() { }

  /// Conversion functions
  const llvm::Type* getType(const hlvm::Type* ty);
  inline const llvm::Type* getFirstClassType(const hlvm::Type* ty);
  llvm::Constant* getConstant(const hlvm::Constant* C);
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

  /// Generator
  template <class NodeClass>
  inline void gen(NodeClass *nc);

  void genProgramLinkage();

  virtual void handleInitialize(AST* tree);
  virtual void handle(Node* n,Pass::TraversalKinds mode);
  virtual void handleTerminate();

  inline llvm::Module* linkModules();
};

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
    case CharacterTypeID:          result = llvm::Type::UIntTy; break;
    case AnyTypeID:
      hlvmNotImplemented("Any Type");
      break;
    case StringTypeID:              
      result = llvm::PointerType::get(llvm::Type::SByteTy);
      break;
    case EnumerationTypeID:
      result = llvm::Type::UIntTy;
      break;
    case IntegerTypeID:
    {
      const IntegerType* IT = llvm::cast<hlvm::IntegerType>(ty);
      uint16_t bits = IT->getBits();
      if (bits <= 8)
        result = (IT->isSigned() ? llvm::Type::SByteTy : llvm::Type::UByteTy);
      else if (bits <= 16)
        result = (IT->isSigned() ? llvm::Type::ShortTy : llvm::Type::UShortTy);
      else if (bits <= 32)
        result = (IT->isSigned() ? llvm::Type::IntTy : llvm::Type::UIntTy);
      else if (bits <= 64)
        result = (IT->isSigned() ? llvm::Type::LongTy : llvm::Type::ULongTy);
      else if (bits <= 128)
        hlvmNotImplemented("128-bit integer");
      else
        hlvmNotImplemented("arbitrary precision integer");
      break;
    }
    case RangeTypeID:
    {
      const RangeType* RT = llvm::cast<hlvm::RangeType>(ty);
      if (RT->getMin() < 0) {
        if (RT->getMin() >= SHRT_MIN && RT->getMax() <= SHRT_MAX)
          return llvm::Type::ShortTy;
        else if (RT->getMin() >= INT_MIN && RT->getMax() <= INT_MAX)
          return llvm::Type::IntTy;
        else
          return llvm::Type::LongTy;
      } else {
        if (RT->getMax() <= USHRT_MAX)
          return llvm::Type::UShortTy;
        else if (RT->getMax() <= UINT_MAX)
          return llvm::Type::UIntTy;
        else
          return llvm::Type::ULongTy;
      }
    }
    case RationalTypeID:
      hlvmNotImplemented("RationalType");
      break;
    case RealTypeID:
    {
      const RealType *RT = llvm::cast<hlvm::RealType>(ty);
      uint16_t bits = RT->getBits();
      if (bits <= 32)
        result = llvm::Type::FloatTy;
      else if (bits <= 64)
        result = llvm::Type::DoubleTy;
      else
        hlvmNotImplemented("arbitrary precision real");
      break;
    }
    case TextTypeID:
      result = em.get_hlvm_text();
      break;
    case StreamTypeID: 
      result = em.get_hlvm_stream();
      break;
    case BufferTypeID: 
      result = em.get_hlvm_buffer();
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
        em.AddType(result, ty->getName());
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
      TypeList params;
      const SignatureType* st = llvm::cast<SignatureType>(ty);
      // Now, push the arguments onto the argument list
      for (SignatureType::const_iterator I = st->begin(), E = st->end(); 
           I != E; ++I)
        params.push_back(getType((*I)->getType()));
      // Get the Result Type
      const llvm::Type* resultTy = getType(st->getResultType());
      result = 
        em.getFunctionType(st->getName(), resultTy, params, st->isVarArgs());
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

const llvm::Type*
LLVMGeneratorPass::getFirstClassType(const hlvm::Type* ty)
{
  const llvm::Type* Ty = getType(ty);
  if (!Ty->isFirstClassType())
    return llvm::PointerType::get(Ty);
  return Ty;
}

llvm::Constant*
LLVMGeneratorPass::getConstant(const hlvm::Constant* C)
{
  hlvmAssert(C!=0);
  hlvmAssert(C->isConstantValue());

  // First, lets see if its cached already
  ConstantMap::iterator I = 
    consts.find(const_cast<hlvm::Constant*>(C));
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
    case ConstantCharacterID:
    {
      const ConstantCharacter* CE = llvm::cast<ConstantCharacter>(C);
      const std::string& cVal = CE->getValue();
      hlvmAssert(!cVal.empty() && "Empty constant character?");
      uint32_t val = 0;
      if (cVal[0] == '#') {
        const char* startptr = &cVal.c_str()[1];
        char* endptr = 0;
        val = strtoul(startptr,&endptr,16);
        hlvmAssert(startptr != endptr);
      } else {
        val = cVal[0];
      }
      result = em.getUVal(llvm::Type::UIntTy,val);
      break;
    }
    case ConstantEnumeratorID:
    {
      const ConstantEnumerator* CE = llvm::cast<ConstantEnumerator>(C);
      const EnumerationType* eType = llvm::cast<EnumerationType>(C->getType());
      uint64_t val = 0;
      bool gotEnumValue = eType->getEnumValue( CE->getValue(), val );
      hlvmAssert(gotEnumValue && "Enumerator not valid for type");
      result = em.getUVal(lType,val);
      break;
    }
    case ConstantIntegerID:
    {
      const ConstantInteger* CI = llvm::cast<const ConstantInteger>(C);
      if (const IntegerType* iType = llvm::dyn_cast<IntegerType>(hType)) {
        if (iType->isSigned()) {
          int64_t val = strtoll(CI->getValue().c_str(),0,CI->getBase());
          result = em.getSVal(lType,val);
        }
        else {
          uint64_t val = strtoull(CI->getValue().c_str(),0,CI->getBase());
          result = em.getUVal(lType,val);
        }
      } else if (const RangeType* rType = llvm::dyn_cast<RangeType>(hType)) {
        int64_t val = strtoll(CI->getValue().c_str(),0,CI->getBase());
        result = em.getSVal(lType,val);
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
      llvm::GlobalVariable* GV  = em.NewGConst(CA->getType(), CA, C->getName());
      std::vector<llvm::Constant*> indices;
      indices.push_back(llvm::Constant::getNullValue(llvm::Type::UIntTy));
      indices.push_back(llvm::Constant::getNullValue(llvm::Type::UIntTy));
      result = llvm::ConstantExpr::getGetElementPtr(GV,indices);
      break;
    }
    case ConstantPointerID:
    {
      const ConstantPointer* hCT = llvm::cast<ConstantPointer>(C);
      const Constant* hC = hCT->getValue();
      const llvm::Type* Ty = getType(hC->getType());
      llvm::Constant* Init = getConstant(hC);
      result = em.NewGConst(Ty,Init, hCT->getName());
      break;
    }
    case ConstantArrayID:
    {
      const ConstantArray* hCA = llvm::cast<ConstantArray>(C);
      const llvm::Type* elemType = getType(hCA->getElementType());
      const llvm::ArrayType* lAT = llvm::ArrayType::get(elemType,hCA->size());
      std::vector<llvm::Constant*> elems;
      for (ConstantArray::const_iterator I = hCA->begin(), E = hCA->end();
           I != E; ++I )
        elems.push_back(getConstant(*I));
      llvm::Constant* lCA = llvm::ConstantArray::get(lAT,elems);
      llvm::GlobalVariable* lGV = em.NewGConst(lAT,lCA,hCA->getName()+"_init");
      llvm::Constant* lCE = em.getFirstElement(lGV);
      const llvm::StructType* Ty = 
        llvm::cast<llvm::StructType>(getType(hCA->getType()));
      elems.clear();
      elems.push_back(em.getUVal(llvm::Type::UIntTy,hCA->size()));
      elems.push_back(lCE);
      result = llvm::ConstantStruct::get(Ty,elems);
      break;
    }
    case ConstantVectorID:
    {
      const ConstantVector* hCA = llvm::cast<ConstantVector>(C);
      const llvm::ArrayType* Ty =
        llvm::cast<llvm::ArrayType>(getType(hCA->getType()));
      std::vector<llvm::Constant*> elems;
      for (ConstantArray::const_iterator I = hCA->begin(), E = hCA->end();
           I != E; ++I )
        elems.push_back(getConstant(*I));
      result = llvm::ConstantArray::get(Ty,elems);
      break;
    }
    case ConstantStructureID:
    {
      const ConstantStructure* hCS = llvm::cast<ConstantStructure>(C);
      const llvm::StructType* Ty = 
        llvm::cast<llvm::StructType>(getType(hCS->getType()));
      std::vector<llvm::Constant*> fields;
      for (ConstantStructure::const_iterator I = hCS->begin(), E = hCS->end(); 
           I != E; ++I)
        fields.push_back(getConstant(*I));
      result = llvm::ConstantStruct::get(Ty,fields);
      break;
    }
    case ConstantContinuationID:
    {
      const ConstantContinuation* hCC = llvm::cast<ConstantContinuation>(C);
      const llvm::StructType* Ty = 
        llvm::cast<llvm::StructType>(getType(hCC->getType()));
      std::vector<llvm::Constant*> fields;
      for (ConstantStructure::const_iterator I = hCC->begin(), E = hCC->end(); 
           I != E; ++I)
        fields.push_back(getConstant(*I));
      // FIXME: Need to add extra fields required for Continuation
      result = llvm::ConstantStruct::get(Ty,fields);
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

  llvm::Value* gv = em.NewGVar(
    /*Ty=*/ getType(V->getType()),
    /*Linkage=*/ getLinkageTypes(V->getLinkageKind()), 
    /*Initializer=*/ Initializer,
    /*Name=*/ getLinkageName(V)
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

  return funcs[F] = em.NewFunction(
    /*Type=*/ llvm::cast<llvm::FunctionType>(getType(F->getType())),
    /*Linkage=*/ getLinkageTypes(F->getLinkageKind()), 
    /*Name=*/ getLinkageName(F)
  );
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
    llvm::Constant* C = llvm::Constant::getNullValue(V->getType());
    return em.emitNE(V,C);
  } else if (llvm::isa<llvm::GlobalValue>(V)) {
    // GlobalValues always have non-zero constant address values, so always true
    return em.getTrue(); 
  }
  hlvmAssert(!"Don't know how to convert V into bool");
  return em.getTrue();
}

llvm::Value* 
LLVMGeneratorPass::ptr2Value(llvm::Value* V)
{
  if (!llvm::isa<llvm::PointerType>(V->getType()))
    return V;

  return em.emitLoad(V,"ptr2Value");
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
    entry_block = exit_block = new llvm::BasicBlock(name,em.getFunction()); 
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

llvm::AllocaInst* 
LLVMGeneratorPass::getOperatorResult(Operator* op, const std::string& name)
{
  llvm::AllocaInst* result = 0;
  if (!llvm::isa<Block>(op->getParent())) {
    const llvm::Type* Ty = getType(op->getType());
    result = em.NewAutoVar(Ty, name + "_var");
    em.emitAssign(result,em.getNullValue(Ty));
  }
  return result;
}

llvm::Value* 
LLVMGeneratorPass::getBlockResult(Block* B)
{
  if (B->getResult() && !em.getBlockTerminator()) {
    llvm::Value* result = operands[B];
    if (llvm::isa<llvm::LoadInst>(result))
      result = llvm::cast<llvm::LoadInst>(result)->getOperand(0);
    result = em.emitLoad(result,em.getBlockName()+"_result");
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
  em.StartFunction(F);
  // Clear the function related variables
  operands.clear();
  enters.clear();
  exits.clear();
  blocks.clear();
  lvars.clear();
}

template<> void
LLVMGeneratorPass::gen(AutoVarOp* av)
{
  // Emit an automatic variable. Note that this is inserted into the entry 
  // block, not the current block, for efficiency. This makes automatic 
  // variables zero cost as well as safeguarding against stack growth if the
  // alloca is in a block that is in a loop.
  const llvm::Type* elemType = getType(av->getType());
  llvm::Value* alloca = em.NewAutoVar(elemType,av->getName()); 
  llvm::Constant* C = 0;
  if (av->hasInitializer())
    C = getConstant(av->getInitializer());
  else
    C = em.getNullValue(elemType);
  em.emitAssign(alloca,C);
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
  pushOperand(em.emitNeg(operand),op);
}

template<> void
LLVMGeneratorPass::gen(ComplementOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  operand = ptr2Value(operand);
  const llvm::Type* lType = operand->getType();
  hlvmAssert(lType->isInteger() && "Can't complement non-integral type");
  pushOperand(em.emitCmpl(operand),op);
}

template<> void
LLVMGeneratorPass::gen(PreIncrOp* op)
{
  llvm::Value* operand = popOperand(op->getOperand(0)); 
  const llvm::Type* lType = operand->getType();
  hlvmAssert(llvm::isa<llvm::PointerType>(lType));
  const llvm::PointerType* PT = llvm::cast<llvm::PointerType>(lType);
  lType = PT->getElementType();
  llvm::LoadInst* load = em.emitLoad(operand,"preincr");
  if (lType->isFloatingPoint()) {
    llvm::Constant* one = em.getFPOne(lType);
    llvm::BinaryOperator* add = em.emitAdd(load,one); 
    pushOperand(em.emitStore(add,operand),op);
  } else if (lType->isInteger()) {
    llvm::Constant* one = em.getIOne(lType);
    llvm::BinaryOperator* add = em.emitAdd(load,one); 
    pushOperand(em.emitStore(add,operand),op);
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
  llvm::LoadInst* load = em.emitLoad(operand,"predecr");
  if (lType->isFloatingPoint()) {
    llvm::Constant* one = em.getFPOne(lType);
    llvm::BinaryOperator* sub = em.emitSub(load,one);
    pushOperand(em.emitStore(sub,operand),op);
  } else if (lType->isInteger()) {
    llvm::Constant* one = em.getIOne(lType);
    llvm::BinaryOperator* sub = em.emitSub(load, one);
    pushOperand(em.emitStore(sub,operand),op);
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
  llvm::LoadInst* load = em.emitLoad(operand,"postincr");
  if (lType->isFloatingPoint()) {
    llvm::Constant* one = em.getFPOne(lType);
    llvm::BinaryOperator* add = em.emitAdd(load,one); 
    em.emitStore(add,operand);
    pushOperand(load,op);
  } else if (lType->isInteger()) {
    llvm::Constant* one = em.getIOne(lType);
    llvm::BinaryOperator* add = em.emitAdd(load,one); 
    em.emitStore(add,operand);
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
  llvm::LoadInst* load = em.emitLoad(operand,"postdecr");
  if (lType->isFloatingPoint()) {
    llvm::Constant* one = em.getFPOne(lType);
    llvm::BinaryOperator* sub = em.emitSub(load, one);
    em.emitStore(sub,operand);
    pushOperand(load,op);
  } else if (lType->isInteger()) {
    llvm::Constant* one = em.getIOne(lType);
    llvm::BinaryOperator* sub = em.emitSub(load, one);
    em.emitStore(sub,operand);
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
  pushOperand(em.emitAdd(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(SubtractOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitSub(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(MultiplyOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitMul(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(DivideOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitDiv(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(ModuloOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitMod(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(BAndOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitBAnd(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(BOrOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitBOr(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(BXorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitBXor(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(BNorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitBNor(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(NotOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  pushOperand(em.emitNot(op1),op);
}

template<> void
LLVMGeneratorPass::gen(AndOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitAnd(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(OrOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitOr(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(NorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitNor(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(XorOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitXor(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(EqualityOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitEQ(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(InequalityOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitNE(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(LessThanOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitLT(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(GreaterThanOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitLT(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(GreaterEqualOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitGE(op1,op2),op);
}

template<> void
LLVMGeneratorPass::gen(LessEqualOp* op)
{
  llvm::Value* op1 = popOperand(op->getOperand(0)); 
  llvm::Value* op2 = popOperand(op->getOperand(1)); 
  op1 = ptr2Value(op1);
  op2 = ptr2Value(op2);
  pushOperand(em.emitLE(op1,op2),op);
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
    pushOperand(em.emitSelect(op1,op2,op3,"select"),op);
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
  em.emitBranch(cond_entry); 

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
  llvm::BasicBlock* select_exit = em.newBlock("select_exit");

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
  em.emitBranch(cond_entry);

  // Get the while loop's body
  llvm::BasicBlock *body_entry, *body_exit;
  llvm::Value* op2 = 
    popOperandAsBlock(op->getOperand(1),"while_body",body_entry,body_exit); 

  // Save the result of the while body, if there should be one
  if (while_result && op2)
    new llvm::StoreInst(op2,while_result,body_exit);

  // Create the exit block
  llvm::BasicBlock* while_exit = em.newBlock("while_exit");

  // Branch the the body block back to the condition branch
  branchIfNotTerminated(cond_entry,body_exit);

  // Finally, install the conditional branch into the branch block
  new llvm::BranchInst(body_entry,while_exit,op1,cond_exit);

  // If there's a result, push it now
  if (while_result)
    pushOperand(new llvm::LoadInst(while_result,"while_result",while_exit),op);

  // Fix up any break or continue operators
  em.ResolveBreaks(while_exit);
  em.ResolveContinues(cond_entry);
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
  em.emitBranch(cond_entry);

  // Get the unless block's body
  llvm::BasicBlock *body_entry, *body_exit;
  llvm::Value* op2 = 
    popOperandAsBlock(op->getOperand(1),"unless_body",body_entry,body_exit); 

  // Save the result of the unless body, if there should be one
  if (unless_result && op2)
    new llvm::StoreInst(op2,unless_result,body_exit);

  // Create the exit block
  llvm::BasicBlock* unless_exit = em.newBlock("unless_exit");

  // Branch the the body block back to the condition branch
  branchIfNotTerminated(cond_entry,body_exit);

  // Finally, install the conditional branch into the branch block
  new llvm::BranchInst(unless_exit,body_entry,op1,cond_exit);

  // If there's a result, push it now
  if (unless_result)
    pushOperand(
      new llvm::LoadInst(unless_result,"unless_result",unless_exit),op);

  // Fix up any break or continue operators
  em.ResolveBreaks(unless_exit);
  em.ResolveContinues(cond_entry);
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
  em.emitBranch(body_entry);

  // Branch the body block to the condition block
  branchIfNotTerminated(cond_entry,body_exit);

  // Create the exit block
  llvm::BasicBlock* until_exit = em.newBlock("until_exit");

  // Finally, install the conditional branch into condition block
  new llvm::BranchInst(until_exit,body_entry,op2,cond_exit);

  // If there's a result, push it now
  if (until_result)
    pushOperand(new llvm::LoadInst(until_result,"until_result",until_exit),op);

  // Fix up any break or continue operators
  em.ResolveBreaks(until_exit);
  em.ResolveContinues(cond_entry);
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
  em.emitBranch(start_cond_entry);

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
  llvm::BasicBlock* loop_exit = em.newBlock("loop_exit");

  // Install the conditional branches for start and end condition blocks
  new llvm::BranchInst(body_entry,loop_exit,op1,start_cond_exit);
  new llvm::BranchInst(start_cond_entry,loop_exit,op3,end_cond_exit);

  // If there's a result, push it now
  if (loop_result)
    pushOperand(new llvm::LoadInst(loop_result,"loop_result",loop_exit),op);

  // Fix up any break or continue operators
  em.ResolveBreaks(loop_exit);
  em.ResolveContinues(end_cond_entry);
}

template<> void
LLVMGeneratorPass::gen(BreakOp* op)
{
  // Make sure the block result is stored
  Block* B = llvm::cast<Block>(op->getContainingBlock());
  getBlockResult(B);

  // Just push a place-holder branch onto the breaks list so it can
  // be fixed up later once we know the destination
  llvm::BranchInst* brnch = em.emitBreak();
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
  llvm::BranchInst* brnch = em.emitContinue();
  pushOperand(brnch,op);
}

template<> void
LLVMGeneratorPass::gen(ResultOp* r)
{
  // Get the result operand
  llvm::Value* src = popOperand(r->getOperand(0));
  // Get the block this result applies to
  hlvm::Block* B = llvm::cast<hlvm::Block>(r->getParent());
  // Get the location into which we will store the result
  llvm::Value* dest = operands[B];

  // Assign the source to the destination
  em.emitAssign(dest, src);
}

template<> void
LLVMGeneratorPass::gen(ReturnOp* r)
{
  // First, if this function returns nothing (void) then just issue a void
  // return instruction.
  const Type* resTy = function->getResultType();
  if (resTy == 0) {
    em.emitReturn(0);
    return;
  }

  // If we get here then the function has a result. 
  llvm::Value* result = operands[function->getBlock()];
  em.emitReturn(result);
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
  pushOperand(em.emitCall(F,args),co);
}

template<> void
LLVMGeneratorPass::gen(StoreOp* s)
{
  llvm::Value* location = popOperand(s->getOperand(0));
  llvm::Value* value =    popOperand(s->getOperand(1));
  // We don't push the StoreInst as an operand because it has no value and
  // therefore cannot be an operand.
  em.emitAssign(location,value);
}

template<> void
LLVMGeneratorPass::gen(LoadOp* l)
{
  llvm::Value* location = popOperand(l->getOperand(0));
  const llvm::PointerType* PT = 
    llvm::dyn_cast<llvm::PointerType>(location->getType());
  hlvmAssert(PT && "Attempt to load non-pointer?");

  const llvm::Type* ET = PT->getElementType();
  if (!ET->isFirstClassType())
    pushOperand(location,l);
  else
    pushOperand(em.emitLoad(location,location->getName()),l);
}

template<> void
LLVMGeneratorPass::gen(ReferenceOp* r)
{
  llvm::Value* referent = getReferent(r);
  pushOperand(referent,r);
}

template<> void
LLVMGeneratorPass::gen(GetFieldOp* i)
{
  llvm::Value* location = popOperand(i->getOperand(0));
  llvm::Value* fld = popOperand(i->getOperand(1));
}

template<> void
LLVMGeneratorPass::gen(GetIndexOp* i)
{
  llvm::Value* location = popOperand(i->getOperand(0));
  llvm::Value* index = popOperand(i->getOperand(1));
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
    ArgList indices;
    em.TwoZeroIndices(indices);
    args.push_back(em.emitGEP(strm,indices));
  } else
    hlvmAssert(!"Array element type is not SByteTy");
  } else
    hlvmAssert(!"OpenOp parameter is not a pointer");
  pushOperand(em.call_hlvm_stream_open(args,"open"),o);
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
    pushOperand(em.call_hlvm_stream_write_string(args,"write"),o);
  if (arg2->getType() == em.get_hlvm_text())
    pushOperand(em.call_hlvm_stream_write_text(args,"write"),o);
  else if (arg2->getType() == em.get_hlvm_buffer())
    pushOperand(em.call_hlvm_stream_write_buffer(args,"write"),o);
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
  pushOperand(em.call_hlvm_stream_read(args,"read"),o);
}

template<> void
LLVMGeneratorPass::gen(CloseOp* o)
{
  llvm::Value* strm = popOperand(o->getOperand(0));
  std::vector<llvm::Value*> args;
  args.push_back(strm);
  pushOperand(em.call_hlvm_stream_close(args),o);
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
  Fields.push_back(llvm::PointerType::get(em.get_hlvm_program_signature()));
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
    llvm::GlobalVariable* name = em.NewGConst(
      /*Type=*/name_val->getType(),
      /*Initializer=*/name_val, 
      /*name=*/"prog_name"
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
    /*Parent=*/em.getModule()
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
        em.StartModule(llvm::cast<Bundle>(n)->getName());
        lvars.clear();
        gvars.clear();
        funcs.clear();
        break;
      }
      case ConstantBooleanID:       
        getConstant(llvm::cast<ConstantBoolean>(n));
        break;
      case ConstantCharacterID:
        getConstant(llvm::cast<ConstantCharacter>(n));
        break;
      case ConstantEnumeratorID:
        getConstant(llvm::cast<ConstantEnumerator>(n));
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
      case ConstantAnyID:
        getConstant(llvm::cast<ConstantAny>(n));
        break;
      case ConstantStructureID:
        getConstant(llvm::cast<ConstantStructure>(n));
        break;
      case ConstantArrayID:
        getConstant(llvm::cast<ConstantArray>(n));
        break;
      case ConstantVectorID:
        getConstant(llvm::cast<ConstantVector>(n));
        break;
      case ConstantContinuationID:
        getConstant(llvm::cast<ConstantContinuation>(n));
        break;
      case ConstantPointerID:
        getConstant(llvm::cast<ConstantPointer>(n));
        break;
      case VariableID:              
        getVariable(llvm::cast<Variable>(n)); 
        break;
      case FunctionID:
      {
        // Get/Create the function
        function = llvm::cast<hlvm::Function>(n);
        startNewFunction(getFunction(function));
        break;
      }
      case ProgramID:
      {
        // Create a new function for the program based on the signature
        function = llvm::cast<hlvm::Program>(n);
        llvm::Function* F = em.NewFunction(
            /*Type=*/ em.get_hlvm_program_signature(),
            /*Linkage=*/llvm::GlobalValue::InternalLinkage,
            /*Name=*/ getLinkageName(function)
        );
        startNewFunction(F);
        // Save the program so it can be generated into the list of program 
        // entry points.
        progs.push_back(F);
        break;
      }
      case BlockID:
      {
        Block* B = llvm::cast<Block>(n);
        std::string name = B->getLabel().empty() ? "block" : B->getLabel();
        enters[B] = em.pushBlock(name);
        if (B->getResult()) {
          const llvm::Type* Ty = getType(B->getType());
          llvm::AllocaInst* result = em.NewAutoVar(Ty, name+"_var");
          // Initialize the autovar to null
          em.emitAssign(result,em.getNullValue(Ty));
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
      case ArrayTypeID:
      case BooleanTypeID:
      case BufferTypeID:
      case CharacterTypeID:
      case EnumerationTypeID:
      case IntegerTypeID:
      case OpaqueTypeID:
      case PointerTypeID:
      case RangeTypeID:
      case RationalTypeID:
      case RealTypeID:
      case StreamTypeID:
      case StringTypeID:
      case StructureTypeID:
      case SignatureTypeID:
      case VectorTypeID:
      {
        Type* t = llvm::cast<Type>(n);
        em.AddType(getType(t), t->getName());
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
      case GetIndexOpID:            gen(llvm::cast<GetIndexOp>(n)); break;
      case GetFieldOpID:            gen(llvm::cast<GetFieldOp>(n)); break;

      case BundleID:
        genProgramLinkage();
        modules.push_back(em.FinishModule());
        break;
      case ConstantBooleanID:       
      case ConstantCharacterID:
      case ConstantEnumeratorID:
      case ConstantIntegerID:      
      case ConstantRealID:        
      case ConstantStringID:     
      case ConstantAnyID:
      case ConstantStructureID:
      case ConstantArrayID:
      case ConstantVectorID:
      case ConstantContinuationID:
      case ConstantPointerID:
        /* FALL THROUGH */
      case VariableID: 
        break;
      case ProgramID:
        /* FALL THROUGH */
      case FunctionID:
        em.FinishFunction();
        // The entry block was created to hold the automatic variables. We now
        // need to terminate the block by branching it to the first active block
        // in the function.
        function = 0;
        break;
      case BlockID:
      {
        Block* B = llvm::cast<Block>(n);
        exits[B] = em.getBlock();
        getBlockResult(B);
        em.popBlock();
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


llvm::cl::opt<bool> NoVerify("no-verify", 
  llvm::cl::init(false),
  llvm::cl::desc("Don't verify generated LLVM module"));

llvm::cl::opt<bool> NoInline("no-inlining", 
  llvm::cl::init(false),
 llvm::cl::desc("Do not run the LLVM inliner pass"));

llvm::cl::opt<bool> NoOptimizations("no-optimization",
  llvm::cl::init(false),
  llvm::cl::desc("Do not run any LLVM optimization passes"));

static void 
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

static bool
optimizeModule(llvm::Module* mod, std::string& ErrMsg)
{
  llvm::PassManager Passes;
  Passes.add(new llvm::TargetData(mod));
  if (!NoVerify)
    if (llvm::verifyModule(*mod,llvm::ReturnStatusAction,&ErrMsg))
      return false;
  getCleanupPasses(Passes);
  Passes.run(*mod);
  if (!NoVerify)
    if (llvm::verifyModule(*mod,llvm::ReturnStatusAction,&ErrMsg))
      return false;
  return true;
}

}

bool
hlvm::generateBytecode(AST* tree, std::ostream& output, std::string& ErrMsg)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  delete PM;
  llvm::Module* mod = genPass.linkModules();
  bool result = optimizeModule(mod,ErrMsg);
  if (result)
    llvm::WriteBytecodeToFile(mod, output, /*compress= */ true);
  delete mod;
  return result;
}

bool
hlvm::generateAssembly(AST* tree, std::ostream& output, std::string& ErrMsg)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  delete PM;
  llvm::Module* mod = genPass.linkModules();
  bool result = optimizeModule(mod,ErrMsg);
  if (result) {
    llvm::PassManager Passes;
    Passes.add(new llvm::PrintModulePass(&output));
    Passes.run(*mod);
  }
  delete mod;
  return result;
}

llvm::Module* 
hlvm::generateModule(AST* tree, std::string& ErrMsg)
{
  hlvm::PassManager* PM = hlvm::PassManager::create();
  LLVMGeneratorPass genPass(tree);
  PM->addPass(&genPass);
  PM->runOn(tree);
  delete PM;
  llvm::Module* mod = genPass.linkModules();
  if (!optimizeModule(mod,ErrMsg)) {
    delete mod;
    return 0;
  }
  return mod;
}

