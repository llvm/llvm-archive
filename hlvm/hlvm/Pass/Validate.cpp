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
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Constants.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/ADT/StringExtras.h>
#include <iostream>
#include <cfloat>
#include <cmath>

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
    inline void error(const Node*n, const std::string& msg);
    inline void warning(const Node*n, const std::string& msg);
    inline bool checkNode(Node*);
    inline bool checkType(Type*,NodeIDs id);
    inline bool checkValue(Value*,NodeIDs id);
    inline bool checkConstant(Constant*,NodeIDs id);
    inline bool checkOperator(
        Operator*,NodeIDs id,unsigned numOps, bool exact = true);
    inline bool checkTerminator(Operator* op);
    inline bool checkUniformContainer(UniformContainerType* T, NodeIDs id);
    inline bool checkDisparateContainer(DisparateContainerType* T, NodeIDs id);
    inline bool checkLinkable(Linkable* LI, NodeIDs id);
    inline bool checkExpression(const Operator* op);
    inline bool checkBooleanExpression(const Operator* op);
    inline bool checkIntegralExpression(const Operator* op);
    inline bool checkResult(const Operator* op);

    template <class NodeClass>
    inline void validate(NodeClass* C);
};

void 
ValidateImpl::warning(const Node* n, const std::string& msg)
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
ValidateImpl::error(const Node* n, const std::string& msg)
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
  if (const ConstantValue* CV = dyn_cast<ConstantValue>(n)) {
    std::cerr << CV->getName() << ": ";
  } else if (const Linkable *L = dyn_cast<Linkable>(n)) {
    std::cerr << L->getName() << ": ";
  } else if (const Type* Ty = dyn_cast<Type>(n)) {
    std::cerr << Ty->getName() << ": ";
  } else if (const NamedType* Ty = dyn_cast<NamedType>(n)) {
    std::cerr << Ty->getName() << ": ";
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
  if (!t->hasName()) {
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
  return result;
}

bool
ValidateImpl::checkConstant(Constant* C,NodeIDs id)
{
  if (checkValue(C,id)) {
    if (!C->hasName() && llvm::isa<Linkable>(C)) {
      error(C,"Linkables must not have an empty name");
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
  else if (n->size() == 0 && !llvm::isa<SignatureType>(n)) {
    error(n,"DisparateContainerType without elements");
    result = false;
  } else {
    for (DisparateContainerType::iterator I = n->begin(), E = n->end(); 
         I != E; ++I) {
      if (!(*I)->getType()) {
        error(n,std::string("Null type not permited in DisparateContainerType")
            + " for '" + (*I)->getName() + "'");
        result = false;
      } else if (!(*I)->hasName()) {
        error((*I)->getType(),"Type has no field name");
        result = false;
      }
    }
  }
  return result;
}

bool 
ValidateImpl::checkLinkable(Linkable* LI, NodeIDs id)
{
  bool result = checkConstant(LI,id);
  if (result) {
    if (LI->getLinkageKind() < ExternalLinkage || 
        LI->getLinkageKind() > InternalLinkage) {
      error(LI,"Invalid LinkageKind for Linkable");
      result = false;
    }
    if (LI->getName().length() == 0)  {
      error(LI,"Zero length name for Linkable");
      result = false;
    }
  }
  return result;
}

bool
ValidateImpl::checkExpression(const Operator* op) 
{
  bool result = true;
  if (const Block* B = dyn_cast<Block>(op)) {
    if (const Operator* block_result = B->getResult()) {
      if (!block_result->getType()) {
        error(op,"Block with void result used where expression expected");
        result = false;
      }
    } else {
      error(op,"Block without result used where expression expected");
      result = false;
    }
    if (B->isTerminated()){
      error(op,"Terminator found in block used as an expression");
      result = false;
    }
  } else if (!op->getType()) {
    error(op,"Operator with void result used where expression expected");
    result = false;
  }
  return result;
}

bool
ValidateImpl::checkBooleanExpression(const Operator* op)
{
  if (checkExpression(op))
    if (!isa<BooleanType>(op->getType())) {
      error(op,std::string("Expecting boolean expression but type '") +
        op->getType()->getName() + "' was found");
      return false;
    } else
      return true;
  return false;
}

bool
ValidateImpl::checkIntegralExpression(const Operator* op)
{
  if (checkExpression(op))
    if (!op->getType()->isIntegralType()) {
      error(op,std::string("Expecting integral expression but type '") +
        op->getType()->getName() + "' was found");
      return false;
    } else
      return true;
  return false;
}

bool 
ValidateImpl::checkResult(const Operator* op)
{
  if (const Block* B = dyn_cast<Block>(op)) {
    if (!B->getResult() && !isa<Block>(B->getParent()->getParent())) {
      error(B,"Block with no result used where an expression is expected");
      return false;
    }
  } else if (!op->getType() && !isa<Block>(op->getParent()->getParent()))  {
    error(op,"Operator with void result used where an expression is expected");
    return false;
  }
  return true;
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
ValidateImpl::validate(IntegerType* n)
{
  if (checkNode(n))
    if (!n->is(IntegerTypeID))
      error(n,"Bad ID for IntegerType");
    else if (n->getBits() == 0)
      error(n,"Integer type cannot have zero bits");
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
    if (!n->is(RealTypeID))
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
ValidateImpl::validate(PointerType* n)
{
  checkUniformContainer(n, PointerTypeID);
}

template<> inline void
ValidateImpl::validate(ArrayType* n)
{
  if (checkUniformContainer(n, ArrayTypeID)) {
    if (n->getMaxSize() == 0)
      error(n,"Array of 0 elements not permited");
  }
}

template<> inline void
ValidateImpl::validate(VectorType* n)
{
  if (checkUniformContainer(n, VectorTypeID)) {
    if (n->getSize() == 0)
      error(n,"Vector of 0 elements not permited");
  }
}

template<> inline void
ValidateImpl::validate(StructureType* n)
{
  if (checkDisparateContainer(n, StructureTypeID)) {
    if (n->size() == 0)
      error(n,"Structure of 0 fields not permited");
  }
}

template<> inline void
ValidateImpl::validate(ContinuationType* n)
{
  if (checkDisparateContainer(n, ContinuationTypeID)) {
    if (n->size() == 0)
      error(n,"Continuation of 0 fields not permited");
  }
}

template<> inline void
ValidateImpl::validate(SignatureType* n)
{
  checkDisparateContainer(n, SignatureTypeID);
}

template<> inline void
ValidateImpl::validate(ConstantAny* n)
{
  checkConstant(n,ConstantAnyID);
}

template<> inline void
ValidateImpl::validate(ConstantBoolean* n)
{
  checkConstant(n,ConstantBooleanID);
}

template<> inline void
ValidateImpl::validate(ConstantCharacter* n)
{
  checkConstant(n,ConstantCharacterID);
}

template<> inline void
ValidateImpl::validate(ConstantEnumerator* n)
{
  if (checkConstant(n,ConstantEnumeratorID)) {
    uint64_t val;
    if (const EnumerationType* ET = dyn_cast<EnumerationType>(n->getType())) {
      if (!ET->getEnumValue( n->getValue(), val))
        error(n, "Enumerator '" + n->getValue() + "' not valid for type '" +
          ET->getName());
    } else {
      error(n,"Type for ConstantEnumerator is not EnumerationType");
    }
  }
}

template<> inline void
ValidateImpl::validate(ConstantInteger* CI)
{
  if (checkConstant(CI,ConstantIntegerID)) {
    const char* startp = CI->getValue().c_str();
    char* endp = 0;
    int64_t val = strtoll(startp,&endp,CI->getBase());
    // Check that it can be converted to binary
    if (!endp || startp == endp || *endp != '\0')
      error(CI,"Invalid integer constant. Conversion failed.");
    else if (const IntegerType* Ty = dyn_cast<IntegerType>(CI->getType())) {
      if (val < 0 && !Ty->isSigned()) {
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
    } else if (const RangeType* Ty = dyn_cast<RangeType>(CI->getType())) {
      if (val < Ty->getMin() || val > Ty->getMax())
        error(CI, "Integer constant out of range of RangeType");
    } else {
      error(CI,"Integer constant applied to non-integer type");
    }
  }
}

template<> inline void
ValidateImpl::validate(ConstantReal* CR)
{
  if (checkConstant(CR,ConstantRealID)) {
    const char* startp = CR->getValue().c_str();
    char* endp = 0;
    long double val = strtold(startp,&endp);
    if (!endp || startp == endp || *endp != '\0')
      error(CR,"Invalid real constant. Conversion failed.");
    else {
      // It converted to a double okay, check that it is in range
      // But, first make sure its not one of the special values
      if (__fpclassify(val) != FP_NORMAL)
        return;
      int exp = ilogbl(val);
      uint64_t val_exp = (exp < 0 ? uint64_t(-exp) : uint64_t(exp));
      unsigned leading_zeros = llvm::CountLeadingZeros_64(val_exp);
      unsigned exp_bits_required = (sizeof(uint64_t)*8 - leading_zeros);
      const RealType* Ty = llvm::cast<RealType>(CR->getType());
      if (Ty->getExponent() < exp_bits_required) {
        error(CR,"Real constant requires too many exponent bits (" +
            llvm::utostr(exp_bits_required) + ") for type '" +
            Ty->getName() + "'");
        return;
      }
      unsigned numBits = Ty->getMantissa() + Ty->getExponent() + 1; 
      if (numBits <= sizeof(float)*8) {
        float x = val;
        long double x2 = x;
        if (val != x2)
          error(CR,"Real constant out of range for real requiring " +
            utostr(numBits) + " bits");
      } 
      else if (numBits <= sizeof(double)*8)
      {
        double x = val;
        long double x2 = x;
        if (val != x2)
          error(CR,"Real constant out of range for real requiring " +
            utostr(numBits) + " bits");
      }
      else if (numBits > sizeof(long double)*8)
        warning(CR,"Don't know how to check range of real type > 128 bits");
    }
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
ValidateImpl::validate(ConstantPointer* n)
{
  if (checkConstant(n,ConstantPointerID)) {
    if (const PointerType* PT = dyn_cast<PointerType>(n->getType())) {
      if (PT->getElementType() != n->getValue()->getType()) {
        error(n,"ConstantPointer referent does not match pointer type");
      }
    } else  {
      error(n,"Constant pointer of type '" + 
          n->getType()->getName() + "' must have pointer type");
    }
  }
}

template<> inline void
ValidateImpl::validate(ConstantArray* n)
{
  if (checkConstant(n,ConstantArrayID)) {
    if (const ArrayType* AT = dyn_cast<ArrayType>(n->getType())) {
      if (n->size() > AT->getMaxSize())
        error(n,"Too many elements (" + utostr(n->size()) + ") for array "
                "of size " + utostr(AT->getMaxSize()));
      else if (n->size() == 0)
        error(n,"Zero sized ConstantArray is not permited");
      for (ConstantArray::iterator I = n->begin(), E = n->end(); I != E; ++I)
        if ((*I)->getType() != AT->getElementType())
          error(n,"Element #" + utostr(I-n->begin()) + "of type '" + 
                   (*I)->getType()->getName() + "' does not conform to array "+
                   "element type (" + AT->getElementType()->getName() + ")");
    } else
      error(n,"ConstantArray must have array type");
  }
}

template<> inline void
ValidateImpl::validate(ConstantVector* n)
{
  if (checkConstant(n,ConstantVectorID)) {
    if (const VectorType* VT = dyn_cast<VectorType>(n->getType())) {
      if (n->size() != VT->getSize())
        error(n,"Too " + 
          (n->size() < VT->getSize() ? std::string("few") :
           std::string("many")) + " elements (" + utostr(n->size()) + 
            ") for array of size " + utostr(VT->getSize()));
      for (ConstantArray::iterator I = n->begin(), E = n->end(); I != E; ++I)
        if ((*I)->getType() != VT->getElementType())
          error(n,"Element #" + utostr(I-n->begin()) + "of type '" + 
                   (*I)->getType()->getName() + "' does not conform to vector "+
                   "element type (" + VT->getElementType()->getName() + ")");
    } else
      error(n,"ConstantVector must have vector type");
  }
}

template<> inline void
ValidateImpl::validate(ConstantStructure* n)
{
  if (checkConstant(n,ConstantStructureID)) {
    if (const StructureType* ST = dyn_cast<StructureType>(n->getType())) {
      if (n->size() != ST->size())
        error(n,"Too " + 
          (n->size() < ST->size() ? std::string("few") :
          std::string("many")) + " elements (" + utostr(n->size()) + 
          ") for structure with " + utostr(ST->size()) + " fields");
      StructureType::const_iterator STI = ST->begin();
      for (ConstantStructure::iterator I = n->begin(), E = n->end(); 
           I != E; ++I) {
        if ((*I)->getType() != (*STI)->getType())
          error(n,"Field #" + utostr(I-n->begin()) + " of type '" 
              + (*I)->getType()->getName() + 
              "' does not conform to type of structure field (" + 
              (*STI)->getName() + ") of type '" + (*STI)->getType()->getName() +
              "'");
        ++STI;
      }
    } else
      error(n,"ConstantStructure must have structure type");
  }
}

template<> inline void
ValidateImpl::validate(ConstantContinuation* n)
{
  if (checkConstant(n,ConstantContinuationID)) {
    if (const ContinuationType* CT = dyn_cast<ContinuationType>(n->getType())) {
      if (n->size() != CT->size())
        error(n,"Too " + 
          (n->size() < CT->size() ? std::string("few") :
          std::string("many")) + " elements (" + utostr(n->size()) + 
          ") for continuation with " + utostr(CT->size()) + " fields");
      ContinuationType::const_iterator CTI = CT->begin();
      for (ConstantContinuation::iterator I = n->begin(), E = n->end(); 
           I != E; ++I) {
        if ((*I)->getType() != (*CTI)->getType())
          error(n,"Field #" + utostr(I-n->begin()) + "of type '" 
              + (*I)->getType()->getName() + 
              "' does not conform to type of continuation field (" + 
              (*CTI)->getName() + ") of type '" + (*CTI)->getType()->getName() +
              "'");
        ++CTI;
      }
    } else
      error(n,"ConstantContinuation must have continuation type");
  }
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
  if (checkLinkable(n, VariableID)) 
    if (n->hasInitializer())
      if (n->getType() != n->getInitializer()->getType())
        error(n,"Variable and its initializer do not agree in type");
}

template<> inline void
ValidateImpl::validate(Function* F)
{
  if (checkLinkable(F, FunctionID)) {
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
  if (checkLinkable(P, ProgramID)) {
    if (P->getSignature() == 0)
      error(P,"Program without signature");
    else if (P->getSignature()->getID() != SignatureTypeID)
      error(P,"Program does not have SignatureType signature");
    else if (P->getSignature() != P->getContainingBundle()->getProgramType())
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
    else {
      Operator* terminator = 0;
      Operator* result = 0;
      for (MultiOperator::iterator I = n->begin(), E = n->end(); I != E; ++I) {
        if (llvm::isa<ResultOp>(*I))
          result = *I;
        else if ((*I)->isTerminator()) {
          if (terminator)
            error(n,"Block contains multiple terminators");
          else
            terminator = *I;
        } else if (terminator)
          error(n,"Block has operators after terminator");
        else if (!llvm::isa<Operator>(*I))
          error(n,"Block contains non-operator");
      }
      if (terminator) {
        if (llvm::isa<ReturnOp>(terminator)) {
          if (!result) {
            Function* F= n->getContainingFunction();
            if (F->getResultType())
              error(terminator,"Missing result in return from function");
          }
        } else if (result) {
          warning(result,"Result will not be used in terminated block");
        }
      } else if (result) {
        if (n == n->getContainingFunction()->getBlock())
          error(result,"Function's main block has result but no return");
      }
    }
}

template<> inline void
ValidateImpl::validate(ResultOp* n)
{
  if (checkOperator(n,ResultOpID,1)) {
    if (Block* B = dyn_cast<Block>(n->getParent())) {
      if (Operator* res = n->getOperand(0)) {
        if (const Operator* existing_result = B->getResult()) {
          if (existing_result->getType() != res->getType()) {
            error(n,"ResultOp does not match block result type");
          }
        } else {
          error(n,"Block has no result type, but has ResultOp");
        }
        if (Function* F = n->getContainingFunction()) {
          if (F->getBlock() == B) {
            const SignatureType* SigTy = F->getSignature();
            if (res->getType()) {
              if (res->getType() != SigTy->getResultType())
                error(n,"ResultOp does not agree in type with Function result");
            } else if (SigTy->getResultType()) {
              error(n,"Void function result for non-void function");
            }
          }
        } else {
          error(n,"ResultOP not in function?");
        }
      } else {
          error(n,"ResultOp with no operand");
      }
    } else {
      error(n, "ResultOp not in a Block");
    }
  }
}

template<> inline void
ValidateImpl::validate(ReturnOp* n)
{
  if (checkOperator(n,ReturnOpID,0))
    checkTerminator(n);
}

template<> inline void
ValidateImpl::validate(BreakOp* n)
{
  if (checkOperator(n,BreakOpID,0))
    if (checkTerminator(n))
      if (!n->getContainingLoop())
        error(n,"Break not within a loop scope");
}

template<> inline void
ValidateImpl::validate(ContinueOp* n)
{
  if (checkOperator(n,ContinueOpID,0))
    if (checkTerminator(n)) 
      if (!n->getContainingLoop())
        error(n,"Continue not within a loop scope");
}

template<> inline void
ValidateImpl::validate(SelectOp* n)
{
  if (checkOperator(n,SelectOpID,3)) {
    checkBooleanExpression(n->getOperand(0));
    if (checkResult(n->getOperand(1)))
      if (checkResult(n->getOperand(2)))
        if (!isa<Block>(n->getParent()))
          if (n->getOperand(1)->getType() != n->getOperand(2)->getType())
            error(n,"SelectOp operands must have same type");
  }
}

template<> inline void
ValidateImpl::validate(WhileOp* n)
{
  if (checkOperator(n,WhileOpID,2)) {
    checkBooleanExpression(n->getOperand(0));
    checkResult(n->getOperand(1));
  }
}

template<> inline void
ValidateImpl::validate(UnlessOp* n)
{
  if (checkOperator(n,UnlessOpID,2)) {
    checkBooleanExpression(n->getOperand(0));
    checkResult(n->getOperand(1));
  }
}

template<> inline void
ValidateImpl::validate(UntilOp* n)
{
  if (checkOperator(n,UntilOpID,2)) {
    checkResult(n->getOperand(0));
    checkBooleanExpression(n->getOperand(1));
  }
}

template<> inline void
ValidateImpl::validate(LoopOp* n)
{
  if (checkOperator(n,LoopOpID,3)) {
    checkBooleanExpression(n->getOperand(0));
    checkResult(n->getOperand(1));
    checkBooleanExpression(n->getOperand(2));
  }
}

template<> inline void
ValidateImpl::validate(SwitchOp* n)
{
  if (checkOperator(n,SwitchOpID,2,false)) 
  {
    if (n->getNumOperands() % 2 != 0)
      error(n,"SwitchOp requires even number of operands");
    else {
      const Type* resultTy = n->getOperand(1)->getType();
      for (SwitchOp::iterator I = n->begin(), E = n->end(); I != E; ++I ) {
        checkIntegralExpression(*I++);
        if (checkResult(*I))
          if (resultTy != (*I)->getType())
            error(*I,"Inconsistent result type for SwitchOp");
      }
    }
  }
}

template<> inline void
ValidateImpl::validate(CallOp* op)
{
  if (checkOperator(op,CallOpID,0,false)) 
  {
    hlvm::Function* F = op->getCalledFunction();
    const SignatureType* sig = F->getSignature();
    if (sig->isVarArgs() && (sig->size() >= op->getNumOperands()))
        error(op,"Too few arguments for varargs function call");
    else if (!sig->isVarArgs() && (sig->size() != op->getNumOperands()-1))
      error(op,"Incorrect number of arguments for function call");
    else 
    {
      unsigned opNum = 1;
      for (SignatureType::const_iterator I = sig->begin(), E = sig->end(); 
           I != E; ++I)
        if ((*I)->getType() != op->getOperand(opNum++)->getType())
          error(op,std::string("Argument #") + utostr(opNum-1) +
              " has wrong type for function");
    }
  }
}

template<> inline void
ValidateImpl::validate(AllocateOp* n)
{
  if (checkOperator(n,AllocateOpID,2)) 
  {
    if (const PointerType* PT = llvm::dyn_cast<PointerType>(n->getType())) 
    {
      if (!PT->getElementType()) 
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
      if (!PT->getElementType())
        error(n,"DeallocateOp's pointer type has no element type!");
    } else
      error(n,"DeallocateOp expects its first operand to be a pointer type");
  }
}

template<> inline void
ValidateImpl::validate(LoadOp* n)
{
  if (checkOperator(n,LoadOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getType();
    if (Ty1 != Ty2)
      error(n,"LoadOp type and operand type do not agree");
  }
}

template<> inline void
ValidateImpl::validate(StoreOp* n)
{
  if (checkOperator(n,StoreOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (Ty1 != Ty2) {
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
  if (checkOperator(op,ReferenceOpID,0,true)) {
    const Value* referent = op->getReferent();
    Block* blk = op->getContainingBlock();
    if (!blk)
      error(op,"ReferenceOp not in a block?");
    else if (const AutoVarOp* ref = dyn_cast<AutoVarOp>(referent)) {
      while (blk != 0) {
        if (AutoVarOp* av = blk->getAutoVar(ref->getName())) 
          if (av == ref)
            break;
        blk = blk->getContainingBlock();
      }
      if (blk == 0)
        error(ref,"Referent does not match name in scope");
    } else if (const Argument* arg = dyn_cast<Argument>(referent)) {
      Function* F = op->getContainingFunction();
      if (!F)
        error(op,"ReferenceOp not in a function?");
      else if (F->getArgument(arg->getName()) != arg)
        error(F,"Referent does not match function argument");
    } else if (const Constant* cval = dyn_cast<Constant>(referent)) {
      Bundle* B = op->getContainingBundle();
      if (!B)
        error(op,"ReferenceOp not in a bundle?");
      else if (B->getConst(cval->getName()) != cval)
        error(cval,"Referent does not value found in Bundle");
    } else {
      error(op,"Referent of unknown kind");
    }
  }
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
    if (!Ty1->is(IntegerTypeID) || !Ty2->is(IntegerTypeID))
      error(n,"You can only bitwise and objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(BOrOp* n)
{
  if (checkOperator(n,BOrOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(IntegerTypeID) || !Ty2->is(IntegerTypeID))
      error(n,"You can only bitwise or objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(BXorOp* n)
{
  if (checkOperator(n,BXorOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(IntegerTypeID) || !Ty2->is(IntegerTypeID))
      error(n,"You can only bitwise xor objects of integer type");
  }
}

template<> inline void
ValidateImpl::validate(BNorOp* n)
{
  if (checkOperator(n,BNorOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(IntegerTypeID) || !Ty2->is(IntegerTypeID))
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
    if (!Ty1->is(RealTypeID))
      error(n,"IsPInfoOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(IsNInfOp* n)
{
  if (checkOperator(n,IsNInfOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"IsPInfoOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(IsNanOp* n)
{
  if (checkOperator(n,IsNanOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"IsNanOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(TruncOp* n)
{
  if (checkOperator(n,TruncOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"TruncOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(RoundOp* n)
{
  if (checkOperator(n,RoundOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"RoundOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(FloorOp* n)
{
  if (checkOperator(n,FloorOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"FloorOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(CeilingOp* n)
{
  if (checkOperator(n,CeilingOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"CeilingOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(LogEOp* n)
{
  if (checkOperator(n,LogEOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"LogEOpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(Log2Op* n)
{
  if (checkOperator(n,Log2OpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"Log2OpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(Log10Op* n)
{
  if (checkOperator(n,Log10OpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"Log10OpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(SquareRootOp* n)
{
  if (checkOperator(n,SquareRootOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"SquareRootOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(CubeRootOp* n)
{
  if (checkOperator(n,CubeRootOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"CubeRootOpID requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(FactorialOp* n)
{
  if (checkOperator(n,FactorialOpID,1)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    if (!Ty1->is(RealTypeID))
      error(n,"FactorialOp requires real number operand");
  }
}

template<> inline void
ValidateImpl::validate(PowerOp* n)
{
  if (checkOperator(n,PowerOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(RealTypeID) || !Ty2->is(RealTypeID))
      error(n,"LogEOpID requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(RootOp* n)
{
  if (checkOperator(n,RootOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(RealTypeID) || !Ty2->is(RealTypeID))
      error(n,"RootOp requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(GCDOp* n)
{
  if (checkOperator(n,GCDOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(RealTypeID) || !Ty2->is(RealTypeID))
      error(n,"GCDOp requires two real number operands");
  }
}

template<> inline void
ValidateImpl::validate(LCMOp* n)
{
  if (checkOperator(n,LCMOpID,2)) {
    const Type* Ty1 = n->getOperand(0)->getType();
    const Type* Ty2 = n->getOperand(1)->getType();
    if (!Ty1->is(RealTypeID) || !Ty2->is(RealTypeID))
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
    case NoNodeID:
      hlvmDeadCode("Invalid Node Kind");
      break;
    case AnyTypeID:              validate(cast<AnyType>(n)); break;
    case BooleanTypeID:          validate(cast<BooleanType>(n)); break;
    case CharacterTypeID:        validate(cast<CharacterType>(n)); break;
    case IntegerTypeID:          validate(cast<IntegerType>(n)); break;
    case RangeTypeID:            validate(cast<RangeType>(n)); break;
    case EnumerationTypeID:      validate(cast<EnumerationType>(n)); break;
    case RealTypeID:             validate(cast<RealType>(n)); break;
    case RationalTypeID:         /*validate(cast<RationalType>(n));*/ break;
    case TextTypeID:             validate(cast<TextType>(n)); break;
    case StringTypeID:           validate(cast<StringType>(n)); break;
    case StreamTypeID:           validate(cast<StreamType>(n)); break;
    case BufferTypeID:           validate(cast<BufferType>(n)); break;
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
    case CallOpID:               validate(cast<CallOp>(n)); break;
    case InvokeOpID:             /*validate(cast<InvokeOp>(n));*/ break;
    case DispatchOpID:           /*validate(cast<DispatchOp>(n));*/ break;
    case CreateContOpID:         /*validate(cast<CreateContOp>(n));*/ break;
    case CallWithContOpID:       /*validate(cast<CallWithContOp>(n));*/ break;
    case ThrowOpID:              /*validate(cast<ThrowOp>(n));*/ break;
    case ReturnOpID:             validate(cast<ReturnOp>(n)); break;
    case ResultOpID:             validate(cast<ResultOp>(n)); break;
    case ContinueOpID:           validate(cast<ContinueOp>(n)); break;
    case BreakOpID:              validate(cast<BreakOp>(n)); break;
    case SelectOpID:             validate(cast<SelectOp>(n)); break;
    case WhileOpID:              validate(cast<WhileOp>(n)); break;
    case UnlessOpID:             validate(cast<UnlessOp>(n)); break;
    case UntilOpID:              validate(cast<UntilOp>(n)); break;
    case LoopOpID:               validate(cast<LoopOp>(n)); break;
    case SwitchOpID:             validate(cast<SwitchOp>(n)); break;
    case LoadOpID:               validate(cast<LoadOp>(n)); break;
    case StoreOpID:              validate(cast<StoreOp>(n)); break;
    case AllocateOpID:           validate(cast<AllocateOp>(n)); break;
    case DeallocateOpID:         validate(cast<DeallocateOp>(n)); break;
    case ReallocateOpID:         /*validate(cast<ReallocateOp>(n));*/ break;
    case ReferenceOpID:          validate(cast<ReferenceOp>(n)); break;
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
    case ConstantAnyID:          validate(cast<ConstantAny>(n)); break;
    case ConstantBooleanID:      validate(cast<ConstantBoolean>(n)); break;
    case ConstantCharacterID:    validate(cast<ConstantCharacter>(n)); break;
    case ConstantEnumeratorID:   validate(cast<ConstantEnumerator>(n)); break;
    case ConstantIntegerID:      validate(cast<ConstantInteger>(n)); break;
    case ConstantRealID:         validate(cast<ConstantReal>(n)); break;
    case ConstantStringID:       validate(cast<ConstantString>(n)); break;
    case ConstantPointerID:      validate(cast<ConstantPointer>(n)); break;
    case ConstantArrayID:        validate(cast<ConstantArray>(n)); break;
    case ConstantVectorID:       validate(cast<ConstantVector>(n)); break;
    case ConstantStructureID:    validate(cast<ConstantStructure>(n)); break;
    case ConstantContinuationID: validate(cast<ConstantContinuation>(n)); break;
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
