//===-- AST Validation Pass -------------------------------------*- C++ -*-===//
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
/// @file hlvm/Pass/Validate.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::Pass::Validate.
//===----------------------------------------------------------------------===//

#include <hlvm/Pass/Pass.h>
#include <hlvm/Base/Assert.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/LinkageItems.h>
#include <hlvm/AST/Constants.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/ADT/StringExtras.h>
#include <iostream>

using namespace hlvm;
using namespace llvm;

namespace {
class ValidateImpl : public Pass
{
  AST* ast;
  public:
    ValidateImpl() : Pass(0,Pass::PostOrderTraversal), ast(0) {}
    virtual void handleInitialize(AST* tree) { ast = tree; }
    virtual void handle(Node* b,Pass::TraversalKinds k);
    inline void error(Node*n, const std::string& msg);
    inline void warning(Node*n, const std::string& msg);
    inline bool checkNode(Node*);
    inline bool checkType(Type*,NodeIDs id);
    inline bool checkValue(Value*,NodeIDs id);
    inline bool checkConstant(Constant*,NodeIDs id);
    inline bool checkOperator(
        Operator*,NodeIDs id,unsigned numOps, bool exact = true);
    inline bool checkTerminator(Operator* op);
    inline bool checkUniformContainer(UniformContainerType* T, NodeIDs id);
    inline bool checkDisparateContainer(DisparateContainerType* T, NodeIDs id);
    inline bool checkLinkageItem(LinkageItem* LI, NodeIDs id);

    template <class NodeClass>
    inline void validate(NodeClass* C);
};

void 
ValidateImpl::warning(Node* n, const std::string& msg)
{
  if (n) {
    const Locator* loc = n->getLocator();
    if (loc) {
      std::string location;
      loc->getLocation(location);
      std::cerr << location << ": ";
    } else
      std::cerr << "Unknown Location: ";
  }
  std::cerr << msg << "\n";
}

void 
ValidateImpl::error(Node* n, const std::string& msg)
{
  if (n) {
    const Locator* loc = n->getLocator();
    if (loc) {
      std::string location;
      loc->getLocation(location);
      std::cerr << location << ": ";
    } else
      std::cerr << "Unknown Location: ";
  }
  std::cerr << msg << "\n";
  passed_ = false;
}


bool
ValidateImpl::checkNode(Node* n)
{
  if (n->getParent() == 0) {
    error(n,"Node has no parent");
    return false;
  }
  if (n->getID() < FirstNodeID || n->getID() > LastNodeID) {
    error(n,"Node ID out of range");
    return false;
  }
  return true;
}

bool
ValidateImpl::checkType(Type* t,NodeIDs id)
{
  bool result = checkNode(t);
  if (t->getID() != id) {
    error(t,"Unexpected NodeID for Type");
    result = false;
  } else if (!t->isType()) {
    error(t,"Bad ID for a Type");
    result = false;
  }
  if (t->getName().empty()) {
    error(t, "Empty type name");
    result = false;
  }
  return result;
}

bool
ValidateImpl::checkValue(Value* v, NodeIDs id)
{
  bool result = checkNode(v);
  if (v->getID() != id) {
    error(v,"Unexpected NodeID for Value");
    result = false;
  } else if (!v->isValue()) {
    error(v,"Bad ID for a Value");
    result = false;
  }
  if (v->getType() == 0) {
    error(v,"Value with no type");
    result = false;
  }
  return result;
}

bool
ValidateImpl::checkConstant(Constant* C,NodeIDs id)
{
  if (checkValue(C,id)) {
    if (C->getName().empty()) {
      error(C,"Constants must not have empty name");
      return false;
    }
  }
  return true;
}

bool
ValidateImpl::checkOperator(Operator*n, NodeIDs id, unsigned num, bool exact)
{
  bool result = checkValue(n,id);
  if (result)
    if (num > n->getNumOperands()) {
      error(n,"Too few operands");
      result = false;
    } else if (exact && num != n->getNumOperands()) {
      error(n, "Too many operands");
      result = false;
    }
  return result;
}

bool
ValidateImpl::checkTerminator(Operator *n)
{
  if (Block* b = llvm::dyn_cast<Block>(n->getParent())) {
    if (b->back() != n) {
      error(n,"Terminating operator is not last operator in block");
      return false;
    }
  } else {
    error(n,"Operator not in block!");
    return false;
  }
  return true;
}

bool 
ValidateImpl::checkUniformContainer(UniformContainerType* n, NodeIDs id)
{
  bool result = true;
  if (!checkType(n,id))
    result = false;
  else if (n->getElementType() == 0) {
    error(n,"UniformContainerType without element type");
    result = false;
  } else if (n->getElementType() == n) {
    error(n,"Self-referential UniformContainerType");
    result = false;
  } else if (n->getName() == n->getElementType()->getName()) {
    error(n,"UniformCOontainerType has same name as its element type");
    result = false;
  }
  return result;
}

bool 
ValidateImpl::checkDisparateContainer(DisparateContainerType* n, NodeIDs id)
{
  bool result = true;
  if (!checkType(n,id))
    result = false;
  else if (n->size() == 0) {
    error(n,"DisparateContainerType without elements");
    result = false;
  } else
    for (DisparateContainerType::iterator I = n->begin(), E = n->end(); 
         I != E; ++I)
      result &= checkUniformContainer(*I, AliasTypeID);
  return result;
}

bool 
ValidateImpl::checkLinkageItem(LinkageItem* LI, NodeIDs id)
{
  bool result = checkConstant(LI,id);
  if (result) {
    if (LI->getLinkageKind() < ExternalLinkage || 
        LI->getLinkageKind() > InternalLinkage) {
      error(LI,"Invalid LinkageKind for LinkageItem");
      result = false;
    }
    if (LI->getName().length() == 0)  {
      error(LI,"Zero length name for LinkageItem");
      result = false;
    }
  }
  return result;
}

template<> inline void
ValidateImpl::validate(VoidType* n)
{
  checkType(n,VoidTypeID);
}

template<> inline void
ValidateImpl::validate(AnyType* n)
{
  checkType(n,AnyTypeID); 
}

template<> inline void
ValidateImpl::validate(BooleanType* n)
{
  checkType(n,BooleanTypeID);
}

template<> inline void
ValidateImpl::validate(CharacterType* n)
{
  checkType(n,CharacterTypeID);
}

template<> inline void
ValidateImpl::validate(OctetType* n)
{
  checkType(n,OctetTypeID);
}

template<> inline void
ValidateImpl::validate(IntegerType* n)
{
  if (checkNode(n))
    if (!n->isIntegerType())
      error(n,"Bad ID for IntegerType");
    else if (n->getBits() == 0)
      error(n,"Invalid number of bits");
}

template<> inline void
ValidateImpl::validate(OpaqueType* n)
{
  checkType(n,OpaqueTypeID);
}

template<> inline void
ValidateImpl::validate(RangeType* n)
{
  if (checkType(n,RangeTypeID))
    if (n->getMin() > n->getMax())
      error(n,"RangeType has negative range");
}

template<> inline void
ValidateImpl::validate(EnumerationType* n)
{
  if (checkType(n,EnumerationTypeID))
    for (EnumerationType::iterator I = n->begin(), E = n->end(); I != E; ++I )
      if ((I)->length() == 0)
        error(n,"Enumerator with zero length");
}

template<> inline void
ValidateImpl::validate(RealType* n)
{
  if (checkNode(n))
    if (!n->isRealType())
      error(n,"Bad ID for RealType");
    else {
      uint64_t bits = n->getMantissa() + n->getExponent();
      if (bits > UINT32_MAX)
        error(n,"Too many bits in RealType");
    }
}

template<> inline void
ValidateImpl::validate(StringType* n)
{
  checkType(n,StringTypeID);
}

template<> inline void
ValidateImpl::validate(TextType* n)
{
  checkType(n,TextTypeID);
}

template<> inline void
ValidateImpl::validate(StreamType* n)
{
  checkType(n,StreamTypeID);
}

template<> inline void
ValidateImpl::validate(BufferType* n)
{
  checkType(n,BufferTypeID);
}

template<> inline void
ValidateImpl::validate(AliasType* n)
{
  checkUniformContainer(n, AliasTypeID);
}

template<> inline void
ValidateImpl::validate(PointerType* n)
{
  checkUniformContainer(n, PointerTypeID);
}

template<> inline void
ValidateImpl::validate(ArrayType* n)
{
  checkUniformContainer(n, ArrayTypeID);
}

template<> inline void
ValidateImpl::validate(VectorType* n)
{
  checkUniformContainer(n, VectorTypeID);
}

template<> inline void
ValidateImpl::validate(StructureType* n)
{
  checkDisparateContainer(n, StructureTypeID);
}

template<> inline void
ValidateImpl::validate(ContinuationType* n)
{
  checkDisparateContainer(n, ContinuationTypeID);
}

template<> inline void
ValidateImpl::validate(SignatureType* n)
{
  checkDisparateContainer(n, SignatureTypeID);
}

template<> inline void
ValidateImpl::validate(ConstantBoolean* n)
{
  checkConstant(n,ConstantBooleanID);
}

template<> inline void
ValidateImpl::validate(ConstantInteger* CI)
{
  if (checkConstant(CI,ConstantIntegerID)) {
    const IntegerType* Ty = cast<IntegerType>(CI->getType());
    // Check that it can be converted to binary
    const char* startp = CI->getValue().c_str();
    char* endp = 0;
    int64_t val = strtoll(startp,&endp,CI->getBase());
    if (!endp || startp == endp || *endp != '\0')
      error(CI,"Invalid integer constant. Conversion failed.");
    else if (val < 0 && !Ty->isSigned()) {
      error(CI,"Invalid integer constant. " 
               "Signed value not accepted by unsigned type");
    } else {
      // It converted to binary okay, check that it is in range
      uint64_t uval = (val < 0) ? -val : val;
      unsigned leading_zeros = llvm::CountLeadingZeros_64(uval);
      unsigned bits_required = (sizeof(uint64_t)*8 - leading_zeros) + 
                               unsigned(Ty->isSigned());
      unsigned bits_allowed  = Ty->getBits();
      if (bits_required > bits_allowed)
        error(CI, "Invalid integer constant. Value requires " + 
              utostr(bits_required) + " bits, but type only holds " +
              utostr(bits_allowed) + " bits.");
    }
  }
}

template<> inline void
ValidateImpl::validate(ConstantReal* CR)
{
  if (checkConstant(CR,ConstantRealID)) {
    const char* startp = CR->getValue().c_str();
    char* endp = 0;
    double val = strtod(startp,&endp);
    if (!endp || startp == endp || *endp != '\0')
      error(CR,"Invalid real constant. Conversion failed.");
  }
}

template<> inline void
ValidateImpl::validate(ConstantString* n)
{
  if (checkConstant(n,ConstantStringID))
    if (std::string::npos != n->getValue().find('\0'))
      error(n,"String constants may not contain a null byte");
}

template<> inline void
ValidateImpl::validate(ConstantAggregate* n)
{
  checkConstant(n,ConstantAggregateID);
  // FIXME: validate fields vs. type
}

template<> inline void
ValidateImpl::validate(ConstantExpression* n)
{
  checkConstant(n,ConstantExpressionID);
  // FIXME: validate opcodes and operands
}

template<> inline void
ValidateImpl::validate(Variable* n)
{
  if (checkLinkageItem(n, VariableID)) 
    if (n->hasInitializer())
      if (n->getType() != n->getInitializer()->getType())
        error(n,"Variable and its initializer do not agree in type");
}

template<> inline void
ValidateImpl::validate(Function* F)
{
  if (checkLinkageItem(F, FunctionID)) {
    if (F->getSignature() == 0)
      error(F,"Function without signature");
    else if (F->getSignature()->getID() != SignatureTypeID)
      error(F,"Function does not have SignatureType signature");
    if (F->getLinkageKind() != ExternalLinkage && F->getBlock() == 0)
      error(F,"Non-external Function without defining block");
  }
}

template<> inline void
ValidateImpl::validate(Program* P)
{
  if (checkLinkageItem(P, ProgramID)) {
    if (P->getSignature() == 0)
      error(P,"Program without signature");
    else if (P->getSignature()->getID() != SignatureTypeID)
      error(P,"Program does not have SignatureType signature");
    else if (P->getSignature() != ast->getProgramType())
      error(P,"Program has wrong signature");
    if (P->getLinkageKind() != ExternalLinkage && P->getBlock() == 0)
      error(P,"Non-external Program without defining block");
  }
}

template<> inline void
ValidateImpl::validate(Block* n)
{
  if (checkValue(n,BlockID))
    if (n->getNumOperands() == 0)
      error(n,"Block with no operands");
    else
      for (MultiOperator::iterator I = n->begin(), E = n->end(); I != E; ++I)
        if (!llvm::isa<Operator>(*I))
          error(n,"Block contains non-operator");
}

template<> inline void
ValidateImpl::validate(NoOperator* n)
{
  checkOperator(n,NoOperatorID,0);
}

template<> inline void
ValidateImpl::validate(ReturnOp* n)
{
  if (checkOperator(n,ReturnOpID,1))
    checkTerminator(n);
}

template<> inline void
ValidateImpl::validate(BreakOp* n)
{
  if (checkOperator(n,BreakOpID,0))
    checkTerminator(n);
}

template<> inline void
ValidateImpl::validate(ContinueOp* n)
{
  if (checkOperator(n,ContinueOpID,0))
    checkTerminator(n);
}

template<> inline void
ValidateImpl::validate(SelectOp* n)
{
  if (checkOperator(n,SelectOpID,3)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<BooleanType>(Ty))
      error(n,"SelectOp expects first operand to be type boolean");
  }
}

template<> inline void
ValidateImpl::validate(LoopOp* n)
{
  if (checkOperator(n,LoopOpID,3)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<BooleanType>(Ty) && !isa<VoidType>(Ty))
      error(n,"LoopOp expects first operand to be type boolean or void");
    Ty = n->getOperand(2)->getType();
    if (!isa<BooleanType>(Ty) && !isa<VoidType>(Ty))
      error(n,"LoopOp expects third operand to be type boolean or void");
  }
}

template<> inline void
ValidateImpl::validate(SwitchOp* n)
{
  if (checkOperator(n,SwitchOpID,2,false)) {
    if (n->getNumOperands() == 2)
      warning(n,"Why not just use a SelectOp?");
    if (n->getNumOperands() % 2 != 0)
      error(n,"SwitchOp requires even number of operands");
  }
}

template<> inline void
ValidateImpl::validate(AllocateOp* n)
{
  if (checkOperator(n,AllocateOpID,2)) {
    if (const PointerType* PT = llvm::dyn_cast<PointerType>(n->getType())) {
      if (const Type* Ty = PT->getElementType()) {
        if (!Ty->isSized())
          error(n,"Can't allocate an unsized type");
      } else 
        error(n,"AllocateOp's pointer type has no element type!");
    } else
      error(n,"AllocateOp's type must be a pointer type");
    if (!llvm::isa<IntegerType>(n->getType())) 
      error(n,"AllocateOp's operand must be of integer type");
  }
}

template<> inline void
ValidateImpl::validate(DeallocateOp* n)
{
  if (checkOperator(n,DeallocateOpID,1)) {
    if (const PointerType* PT = llvm::dyn_cast<PointerType>(n->getType())) {
      if (const Type* Ty = PT->getElementType()) {
        if (!Ty->isSized())
          error(n,"Can't deallocate an unsized type");
      } else 
        error(n,"DeallocateOp's pointer type has no element type!");
    } else
      error(n,"DeallocateOp expects its first operand to be a pointer type");
  }
}

template<> inline void
ValidateImpl::validate(LoadOp* n)
{
  if (checkOperator(n,LoadOpID,1)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<PointerType>(Ty))
      error(n,"LoadOp expects a pointer type operand");
    else if (n->getType() != cast<PointerType>(Ty)->getElementType())
      error(n,"LoadOp type and operand type do not agree");
  }
}

template<> inline void
ValidateImpl::validate(StoreOp* n)
{
  if (checkOperator(n,StoreOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!isa<PointerType>(Ty1))
      error(n,"StoreOp expects first operand to be pointer type");
    else if (cast<PointerType>(Ty1)->getElementType() != Ty2) {
      error(n,"StoreOp operands do not agree in type");
    } else if (const ReferenceOp* ref = 
               dyn_cast<ReferenceOp>(n->getOperand(0))) {
      const Value* R = ref->getReferent();
      if (isa<Variable>(R) && cast<Variable>(R)->isConstant())
        error(n,"Can't store to constant variable");
      else if (isa<AutoVarOp>(R) && cast<AutoVarOp>(R)->isConstant())
        error(n,"Can't store to constant automatic variable");
    } else if (const IndexOp* ref = dyn_cast<IndexOp>(n)) {
      /// FIXME: Implement this
    }
  }
}

template<> inline void
ValidateImpl::validate(AutoVarOp* n)
{
  if (checkOperator(n,AutoVarOpID,0,false)) {
    if (n->hasInitializer()) {
      if (n->getType() != n->getInitializer()->getType()) {
        error(n,"AutoVarOp's type does not agree with type of its Initializer");
      } 
    }
  }
}

template<> inline void
ValidateImpl::validate(ReferenceOp* op)
{
  checkOperator(op,ReferenceOpID,0,true);
  /// FIXME: check referent out
}

template<> inline void
ValidateImpl::validate(ConstantReferenceOp* op)
{
  checkOperator(op,ConstantReferenceOpID,0,true);
  /// FIXME: check referent out
}

template<> inline void
ValidateImpl::validate(NegateOp* n)
{
  if (checkOperator(n,NegateOpID,1)) {
    const Type* Ty = n->getType();
    if (!Ty->isNumericType())
      error(n,"You can only negate objects of numeric type");
  }
}

template<> inline void
ValidateImpl::validate(ComplementOp* n)
{
  if (checkOperator(n,ComplementOpID,1)) {
    const Type* Ty = n->getType();
    if (!Ty->isIntegralType())
      error(n,"You can only complement objects of integral type");
  }
}

template<> inline void
ValidateImpl::validate(PreIncrOp* n)
{
  if (checkOperator(n,PreIncrOpID,1)) {
    if (ReferenceOp* oprnd = llvm::dyn_cast<ReferenceOp>(n->getOperand(0))) {
      const Value* V = oprnd->getReferent();
      if (V && (isa<AutoVarOp>(V) || isa<Variable>(V))) {
        if (!V->getType()->isNumericType())
          error(n,"Target of PostIncrOp is not numeric type");
      } else
        ; // Handled elsewhere
    } else 
      error(n,"Operand of PostIncrOp must be a ReferenceOp");
  }
}

template<> inline void
ValidateImpl::validate(PostIncrOp* n)
{
  if (checkOperator(n,PostIncrOpID,1)) {
    if (ReferenceOp* oprnd = llvm::dyn_cast<ReferenceOp>(n->getOperand(0))) {
      const Value* V = oprnd->getReferent();
      if (V && (isa<AutoVarOp>(V) || isa<Variable>(V))) {
        if (!V->getType()->isNumericType())
          error(n,"Target of PostIncrOp is not numeric type");
      } else
        ; // Handled elsewhere
    } else 
      error(n,"Operand of PostIncrOp must be a ReferenceOp");
  }
}

template<> inline void
ValidateImpl::validate(PreDecrOp* n)
{
  if (checkOperator(n,PreDecrOpID,1)){
    if (ReferenceOp* oprnd = llvm::dyn_cast<ReferenceOp>(n->getOperand(0))) {
      const Value* V = oprnd->getReferent();
      if (V && (isa<AutoVarOp>(V) || isa<Variable>(V))) {
        if (!V->getType()->isNumericType())
          error(n,"Target of PreDecrOp is not numeric type");
      } else
        ; // Handled elsewhere
    } else 
      error(n,"Operand of PreDecrOp must be a ReferenceOp");
  }
}

template<> inline void
ValidateImpl::validate(PostDecrOp* n)
{
  if (checkOperator(n,PostDecrOpID,1)) {
    if (ReferenceOp* oprnd = llvm::dyn_cast<ReferenceOp>(n->getOperand(0))) {
      const Value* V = oprnd->getReferent();
      if (V && (isa<AutoVarOp>(V) || isa<Variable>(V))) {
        if (!V->getType()->isNumericType())
          error(n,"Target of PostDecrOp is not numeric type");
      } else
        ; // Handled elsewhere
    } else 
      error(n,"Operand of PostDecrOp must be a ReferenceOp");
  }
}

template<> inline void
ValidateImpl::validate(AddOp* n)
{
  if (checkOperator(n,AddOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isNumericType() || !Ty2->isNumericType())
      error(n,"You can only add objects of numeric type");
  }
}

template<> inline void
ValidateImpl::validate(SubtractOp* n)
{
  if (checkOperator(n,SubtractOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isNumericType() || !Ty2->isNumericType())
      error(n,"You can only subtract objects of numeric type");
  }
}

template<> inline void
ValidateImpl::validate(MultiplyOp* n)
{
  if (checkOperator(n,MultiplyOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isNumericType() || !Ty2->isNumericType())
      error(n,"You can only multiply objects of numeric type");
  }
}

template<> inline void
ValidateImpl::validate(DivideOp* n)
{
  if (checkOperator(n,DivideOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isNumericType() || !Ty2->isNumericType())
      error(n,"You can only multiply objects of numeric type");
  }
}

template<> inline void
ValidateImpl::validate(ModuloOp* n)
{
  if (checkOperator(n,ModuloOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isNumericType() || !Ty2->isNumericType())
      error(n,"You can only multiply objects of numeric type");
  }
}

template<> inline void
ValidateImpl::validate(BAndOp* n)
{
  if (checkOperator(n,BAndOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isIntegerType() || !Ty2->isIntegerType())
      error(n,"You can only bitwise and objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(BOrOp* n)
{
  if (checkOperator(n,BOrOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isIntegerType() || !Ty2->isIntegerType())
      error(n,"You can only bitwise or objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(BXorOp* n)
{
  if (checkOperator(n,BXorOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isIntegerType() || !Ty2->isIntegerType())
      error(n,"You can only bitwise xor objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(BNorOp* n)
{
  if (checkOperator(n,BNorOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isIntegerType() || !Ty2->isIntegerType())
      error(n,"You can only bitwise nor objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(NotOp* n)
{
  if (checkOperator(n,NotOpID,1)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!Ty->is(BooleanTypeID))
      error(n,"NotOp expects boolean typed operand");
  }
}

template<> inline void
ValidateImpl::validate(AndOp* n)
{
  if (checkOperator(n,AndOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(BooleanTypeID) || !Ty2->is(BooleanTypeID))
      error(n,"AndOp expects two boolean typed operands");
  }
}

template<> inline void
ValidateImpl::validate(OrOp* n)
{
  if (checkOperator(n,OrOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(BooleanTypeID) || !Ty2->is(BooleanTypeID))
      error(n,"OrOp expects two boolean typed operands");
  }
}

template<> inline void
ValidateImpl::validate(NorOp* n)
{
  if (checkOperator(n,NorOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(BooleanTypeID) || !Ty2->is(BooleanTypeID))
      error(n,"NorOp expects two boolean typed operands");
  }
}

template<> inline void
ValidateImpl::validate(XorOp* n)
{
  if (checkOperator(n,XorOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(BooleanTypeID) || !Ty2->is(BooleanTypeID))
      error(n,"XorOp expects two boolean typed operands");
  }
}

template<> inline void
ValidateImpl::validate(LessThanOp* n)
{
  if (checkOperator(n,LessThanOpID,2))
    ;
}

template<> inline void
ValidateImpl::validate(GreaterThanOp* n)
{
  if (checkOperator(n,GreaterThanOpID,2))
    ;
}

template<> inline void
ValidateImpl::validate(LessEqualOp* n)
{
  if (checkOperator(n,LessEqualOpID,2))
    ;
}

template<> inline void
ValidateImpl::validate(GreaterEqualOp* n)
{
  if (checkOperator(n,GreaterEqualOpID,2))
    ;
}

template<> inline void
ValidateImpl::validate(EqualityOp* n)
{
  if (checkOperator(n,EqualityOpID,2))
    ;
}

template<> inline void
ValidateImpl::validate(InequalityOp* n)
{
  if (checkOperator(n,InequalityOpID,2))
    ;
}

template<> inline void
ValidateImpl::validate(IsPInfOp* n)
{
  if (checkOperator(n,IsPInfOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"IsPInfoOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(IsNInfOp* n)
{
  if (checkOperator(n,IsNInfOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"IsPInfoOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(IsNanOp* n)
{
  if (checkOperator(n,IsNanOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"IsNanOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(TruncOp* n)
{
  if (checkOperator(n,TruncOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"TruncOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(RoundOp* n)
{
  if (checkOperator(n,RoundOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"RoundOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(FloorOp* n)
{
  if (checkOperator(n,FloorOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"FloorOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(CeilingOp* n)
{
  if (checkOperator(n,CeilingOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"CeilingOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(LogEOp* n)
{
  if (checkOperator(n,LogEOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"LogEOpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(Log2Op* n)
{
  if (checkOperator(n,Log2OpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"Log2OpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(Log10Op* n)
{
  if (checkOperator(n,Log10OpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"Log10OpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(SquareRootOp* n)
{
  if (checkOperator(n,SquareRootOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"SquareRootOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(CubeRootOp* n)
{
  if (checkOperator(n,CubeRootOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"CubeRootOpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(FactorialOp* n)
{
  if (checkOperator(n,FactorialOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->isRealType())
      error(n,"FactorialOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(PowerOp* n)
{
  if (checkOperator(n,PowerOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isRealType() || !Ty2->isRealType())
      error(n,"LogEOpID requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(RootOp* n)
{
  if (checkOperator(n,RootOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isRealType() || !Ty2->isRealType())
      error(n,"RootOp requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(GCDOp* n)
{
  if (checkOperator(n,GCDOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isRealType() || !Ty2->isRealType())
      error(n,"GCDOp requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(LCMOp* n)
{
  if (checkOperator(n,LCMOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->isRealType() || !Ty2->isRealType())
      error(n,"LCMOp requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(OpenOp* n)
{
  if (checkOperator(n,OpenOpID,1)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<StringType>(Ty))
      error(n,"OpenOp expects first operand to be a string");
  }
}

template<> inline void
ValidateImpl::validate(CloseOp* n)
{
  if (checkOperator(n,CloseOpID,1)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<StreamType>(Ty))
      error(n,"CloseOp expects first operand to be a stream");
  }
}

template<> inline void
ValidateImpl::validate(ReadOp* n)
{
  if (checkOperator(n,ReadOpID,2)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<StreamType>(Ty))
      error(n,"ReadOp expects first operand to be a stream");
    Ty = n->getOperand(1)->getType();
    if (!isa<BufferType>(Ty))
      error(n,"ReadOp expects second operand to be a buffer");
  }
}

template<> inline void
ValidateImpl::validate(WriteOp* n)
{
  if (checkOperator(n,WriteOpID,2)) {
    const Type* Ty = n->getOperand(0)->getType();
    if (!isa<StreamType>(Ty))
      error(n,"WriteOp expects first operand to be a stream");
    Ty = n->getOperand(1)->getType();
    switch (Ty->getID()) {
      case BufferTypeID:
      case TextTypeID:
      case StringTypeID:
        break;
      default:
        error(n,"WriteOp expects second operand to be a writable type");
        break;
    }
  }
}

template<> inline void
ValidateImpl::validate(Bundle* n)
{
  // FIXME: checkNode(n);
}

template<> inline void
ValidateImpl::validate(Import* n)
{
  // FIXME: checkNode(n);
}

void
ValidateImpl::handle(Node* n,Pass::TraversalKinds k)
{
  switch (n->getID())
  {
    case NoTypeID:
      hlvmDeadCode("Invalid Node Kind");
      break;
    case VoidTypeID:             validate(cast<VoidType>(n)); break;
    case AnyTypeID:              validate(cast<AnyType>(n)); break;
    case BooleanTypeID:          validate(cast<BooleanType>(n)); break;
    case CharacterTypeID:        validate(cast<CharacterType>(n)); break;
    case OctetTypeID:            validate(cast<OctetType>(n)); break;
    case IntegerTypeID:          validate(cast<IntegerType>(n)); break;
    case RangeTypeID:            validate(cast<RangeType>(n)); break;
    case EnumerationTypeID:      validate(cast<EnumerationType>(n)); break;
    case RealTypeID:             validate(cast<RealType>(n)); break;
    case RationalTypeID:         /*validate(cast<RationalType>(n));*/ break;
    case TextTypeID:             validate(cast<TextType>(n)); break;
    case StreamTypeID:           validate(cast<StreamType>(n)); break;
    case BufferTypeID:           validate(cast<BufferType>(n)); break;
    case AliasTypeID:            validate(cast<AliasType>(n)); break;
    case PointerTypeID:          validate(cast<PointerType>(n)); break;
    case ArrayTypeID:            validate(cast<ArrayType>(n)); break;
    case VectorTypeID:           validate(cast<VectorType>(n)); break;
    case StructureTypeID:        validate(cast<StructureType>(n)); break;
    case SignatureTypeID:        validate(cast<SignatureType>(n)); break;
    case ContinuationTypeID:     validate(cast<ContinuationType>(n)); break;
    case OpaqueTypeID:           validate(cast<OpaqueType>(n)); break;
    case InterfaceID:            /*validate(cast<InterfaceID>);*/ break;
    case ClassID:                /*validate(cast<ClassID>);*/ break;
    case MethodID:               /*validate(cast<MethodID>);*/ break;
    case ImplementsID:           /*validate(cast<ImplementsID>);*/ break;
    case VariableID:             validate(cast<Variable>(n)); break;
    case FunctionID:             validate(cast<Function>(n)); break;
    case ProgramID:              validate(cast<Program>(n)); break;
    case BundleID:               validate(cast<Bundle>(n)); break;
    case BlockID:                validate(cast<Block>(n)); break;
    case ImportID:               validate(cast<Import>(n)); break;
    case CallOpID:               /*validate(cast<CallOp>(n));*/ break;
    case InvokeOpID:             /*validate(cast<InvokeOp>(n));*/ break;
    case DispatchOpID:           /*validate(cast<DispatchOp>(n));*/ break;
    case CreateContOpID:         /*validate(cast<CreateContOp>(n));*/ break;
    case CallWithContOpID:       /*validate(cast<CallWithContOp>(n));*/ break;
    case ThrowOpID:              /*validate(cast<ThrowOp>(n));*/ break;
    case NoOperatorID:           validate(cast<NoOperator>(n)); break;
    case ReturnOpID:             validate(cast<ReturnOp>(n)); break;
    case ContinueOpID:           validate(cast<ContinueOp>(n)); break;
    case BreakOpID:              validate(cast<BreakOp>(n)); break;
    case SelectOpID:             validate(cast<SelectOp>(n)); break;
    case LoopOpID:               validate(cast<LoopOp>(n)); break;
    case SwitchOpID:             validate(cast<SwitchOp>(n)); break;
    case LoadOpID:               validate(cast<LoadOp>(n)); break;
    case StoreOpID:              validate(cast<StoreOp>(n)); break;
    case AllocateOpID:           validate(cast<AllocateOp>(n)); break;
    case DeallocateOpID:         validate(cast<DeallocateOp>(n)); break;
    case ReallocateOpID:         /*validate(cast<ReallocateOp>(n));*/ break;
    case ReferenceOpID:          validate(cast<ReferenceOp>(n)); break;
    case ConstantReferenceOpID:  validate(cast<ConstantReferenceOp>(n)); 
                                                                     break;
    case AutoVarOpID:            validate(cast<AutoVarOp>(n)); break;
    case NegateOpID:             validate(cast<NegateOp>(n)); break;
    case ComplementOpID:         validate(cast<ComplementOp>(n)); break;
    case PreIncrOpID:            validate(cast<PreIncrOp>(n)); break;
    case PostIncrOpID:           validate(cast<PostIncrOp>(n)); break;
    case PreDecrOpID:            validate(cast<PreDecrOp>(n)); break;
    case PostDecrOpID:           validate(cast<PostDecrOp>(n)); break;
    case AddOpID:                validate(cast<AddOp>(n)); break;
    case SubtractOpID:           validate(cast<SubtractOp>(n)); break;
    case MultiplyOpID:           validate(cast<MultiplyOp>(n)); break;
    case DivideOpID:             validate(cast<DivideOp>(n)); break;
    case ModuloOpID:             validate(cast<ModuloOp>(n)); break;
    case BAndOpID:               validate(cast<BAndOp>(n)); break;
    case BOrOpID:                validate(cast<BOrOp>(n)); break;
    case BXorOpID:               validate(cast<BXorOp>(n)); break;
    case BNorOpID:               validate(cast<BNorOp>(n)); break;
    case NotOpID:                validate(cast<NotOp>(n)); break;
    case AndOpID:                validate(cast<AndOp>(n)); break;
    case OrOpID:                 validate(cast<OrOp>(n)); break;
    case NorOpID:                validate(cast<NorOp>(n)); break;
    case XorOpID:                validate(cast<XorOp>(n)); break;
    case LessThanOpID:           validate(cast<LessThanOp>(n)); break;
    case GreaterThanOpID:        validate(cast<GreaterThanOp>(n)); break;
    case LessEqualOpID:          validate(cast<LessEqualOp>(n)); break;
    case GreaterEqualOpID:       validate(cast<GreaterEqualOp>(n)); break;
    case EqualityOpID:           validate(cast<EqualityOp>(n)); break;
    case InequalityOpID:         validate(cast<InequalityOp>(n)); break;
    case IsPInfOpID:             validate(cast<IsPInfOp>(n)); break;
    case IsNInfOpID:             validate(cast<IsNInfOp>(n)); break;
    case IsNanOpID:              validate(cast<IsNanOp>(n)); break;
    case TruncOpID:              validate(cast<TruncOp>(n)); break;
    case RoundOpID:              validate(cast<RoundOp>(n)); break;
    case FloorOpID:              validate(cast<FloorOp>(n)); break;
    case CeilingOpID:            validate(cast<CeilingOp>(n)); break;
    case LogEOpID:               validate(cast<LogEOp>(n)); break;
    case Log2OpID:               validate(cast<Log2Op>(n)); break;
    case Log10OpID:              validate(cast<Log10Op>(n)); break;
    case SquareRootOpID:         validate(cast<SquareRootOp>(n)); break;
    case CubeRootOpID:           validate(cast<CubeRootOp>(n)); break;
    case FactorialOpID:          validate(cast<FactorialOp>(n)); break;
    case PowerOpID:              validate(cast<PowerOp>(n)); break;
    case RootOpID:               validate(cast<RootOp>(n)); break;
    case GCDOpID:                validate(cast<GCDOp>(n)); break;
    case LCMOpID:                validate(cast<LCMOp>(n)); break;
    case LengthOpID:             /*validate(cast<LengthOp>(n)); */ break;
    case OpenOpID:               validate(cast<OpenOp>(n)); break;
    case CloseOpID:              validate(cast<CloseOp>(n)); break;
    case ReadOpID:               validate(cast<ReadOp>(n)); break;
    case WriteOpID:              validate(cast<WriteOp>(n)); break;
    case PositionOpID:           /*validate(cast<PositionOp>(n));*/ break;
    case PInfOpID:               /*validate(cast<PInfOp>(n)); */ break;
    case NInfOpID:               /*validate(cast<NInfoOp>(n)); */ break;
    case NaNOpID:                /*validate(cast<NaNOp>(n)); */ break;
    case ConstantBooleanID:      validate(cast<ConstantBoolean>(n)); break;
    case ConstantIntegerID:      validate(cast<ConstantInteger>(n)); break;
    case ConstantRealID:         validate(cast<ConstantReal>(n)); break;
    case ConstantStringID:       validate(cast<ConstantString>(n)); break;
    case ConstantAggregateID:    validate(cast<ConstantAggregate>(n)); break;
    case ConstantExpressionID:   validate(cast<ConstantExpression>(n)); break;
      break; // Not implemented yet
    case DocumentationID:
      /// Nothing to validate (any doc is a good thing :)
      break;
    default:
      hlvmDeadCode("Invalid Node Kind");
      break;
    }
}

}

Pass* 
Pass::new_ValidatePass()
{
  return new ValidateImpl();
}

bool
hlvm::validate(AST* tree)
{
  PassManager* PM = PassManager::create();
  Pass* pass = Pass::new_ValidatePass(); 
  PM->addPass( pass );
  PM->runOn(tree);
  delete PM;
  bool result = pass->passed();
  delete pass;
  return result;
}
