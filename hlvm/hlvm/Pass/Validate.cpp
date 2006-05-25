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
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Variable.h>
#include <hlvm/AST/Program.h>
#include <llvm/Support/Casting.h>
#include <iostream>

using namespace hlvm;
using namespace llvm;

namespace {
class ValidateImpl : public Pass
{
  public:
    ValidateImpl() : Pass(0,Pass::PostOrderTraversal) {}
    virtual void handle(Node* b,Pass::TraversalKinds k);
    inline void error(Node*n, const char* msg);
    inline void validateName(NamedNode* n);
    inline void validateIntegerType(IntegerType* n);
    inline void validateRangeType(RangeType* n);
    inline void validateEnumerationType(EnumerationType* n);
    inline void validateRealType(RealType* n);
    inline void validateAliasType(AliasType* n);
    inline void validatePointerType(PointerType* n);
    inline void validateArrayType(ArrayType* n);
    inline void validateVectorType(VectorType* n);
    inline void validateStructureType(StructureType* n);
    inline void validateSignatureType(SignatureType* n);
    inline void validateVariable(Variable* n);
    inline void validateFunction(Variable* n);
    inline void validateProgram(Program* n);
    inline void validateBundle(Bundle* n);
};

void 
ValidateImpl::error(Node* n, const char* msg)
{
  std::cerr << "Node(" << n << "): " << msg << "\n";
}

inline void
ValidateImpl::validateName(NamedNode* n)
{
  const std::string& name = n->getName();
  if (name.empty()) {
    error(n,"Empty Name");
  }
}
inline void
ValidateImpl::validateIntegerType(IntegerType* n)
{
  validateName(n);
  if (n->getBits() == 0) {
    error(n,"Invalid number of bits");
  }
}

inline void
ValidateImpl::validateRangeType(RangeType* n)
{
}

inline void
ValidateImpl::validateEnumerationType(EnumerationType* n)
{
}

inline void
ValidateImpl::validateRealType(RealType* n)
{
}

inline void
ValidateImpl::validateAliasType(AliasType* n)
{
}

inline void
ValidateImpl::validatePointerType(PointerType* n)
{
}

inline void
ValidateImpl::validateArrayType(ArrayType* n)
{
}

inline void
ValidateImpl::validateVectorType(VectorType* n)
{
}

inline void
ValidateImpl::validateStructureType(StructureType* n)
{
}

inline void
ValidateImpl::validateSignatureType(SignatureType* n)
{
}

inline void
ValidateImpl::validateVariable(Variable* n)
{
}

inline void
ValidateImpl::validateFunction(Variable* n)
{
}

inline void
ValidateImpl::validateProgram(Program* n)
{
}

inline void
ValidateImpl::validateBundle(Bundle* n)
{
}

void
ValidateImpl::handle(Node* n,Pass::TraversalKinds k)
{
  switch (n->getID())
  {
    case NoTypeID:
      hlvmDeadCode("Invalid Node Kind");
      break;
    case VoidTypeID:
    case AnyTypeID:
    case BooleanTypeID:
    case CharacterTypeID:
    case OctetTypeID:
      validateName(cast<NamedNode>(n));
      break; 
    case IntegerTypeID:
      validateIntegerType(cast<IntegerType>(n));
      break;
    case RangeTypeID:
      validateRangeType(cast<RangeType>(n));
      break;
    case EnumerationTypeID:
      validateEnumerationType(cast<EnumerationType>(n));
      break;
    case RealTypeID:
      validateRealType(cast<RealType>(n));
      break;
    case RationalTypeID:
      break; // Not implemented yet
    case StringTypeID:
      break; // Not imlpemented yet
    case AliasTypeID:
      validateAliasType(cast<AliasType>(n));
      break;
    case PointerTypeID:
      validatePointerType(cast<PointerType>(n));
      break;
    case ArrayTypeID:
      validateArrayType(cast<ArrayType>(n));
      break;
    case VectorTypeID:
      validateVectorType(cast<VectorType>(n));
      break;
    case StructureTypeID:
      validateStructureType(cast<StructureType>(n));
      break;
    case SignatureTypeID:
      validateSignatureType(cast<SignatureType>(n));
      break;
    case ContinuationTypeID:
    case OpaqueTypeID:
    case InterfaceID:
    case ClassID:
    case MethodID:
    case ImplementsID:
      break; // Not implemented yet
    case VariableID:
      validateVariable(cast<Variable>(n));
      break;
    case FunctionID:
      validateFunction(cast<Variable>(n));
      break;
    case ProgramID:
      validateProgram(cast<Program>(n));
      break;
    case BundleID:
      validateBundle(cast<Bundle>(n));
      break;
    case BlockID:
    case ImportID:
    case CallOpID:
    case InvokeOpID:
    case DispatchOpID:
    case CreateContOpID:
    case CallWithContOpID:
    case ReturnOpID:
    case ThrowOpID:
    case JumpToOpID:
    case BreakOpID:
    case IfOpID:
    case LoopOpID:
    case SelectOpID:
    case WithOpID:
    case LoadOpID:
    case StoreOpID:
    case AllocateOpID:
    case FreeOpID:
    case ReallocateOpID:
    case StackAllocOpID:
    case ReferenceOpID:
    case DereferenceOpID:
    case NegateOpID:
    case ComplementOpID:
    case PreIncrOpID:
    case PostIncrOpID:
    case PreDecrOpID:
    case PostDecrOpID:
    case AddOpID:
    case SubtractOpID:
    case MultiplyOpID:
    case DivideOpID:
    case ModulusOpID:
    case BAndOpID:
    case BOrOpID:
    case BXOrOpID:
    case AndOpID:
    case OrOpID:
    case NorOpID:
    case XorOpID:
    case NotOpID:
    case LTOpID:
    case GTOpID:
    case LEOpID:
    case GEOpID:
    case EQOpID:
    case NEOpID:
    case IsPInfOpID:
    case IsNInfOpID:
    case IsNaNOpID:
    case TruncOpID:
    case RoundOpID:
    case FloorOpID:
    case CeilingOpID:
    case PowerOpID:
    case LogEOpID:
    case Log2OpID:
    case Log10OpID:
    case SqRootOpID:
    case RootOpID:
    case FactorialOpID:
    case GCDOpID:
    case LCMOpID:
    case LengthOpID:
    case OpenOpID:
    case CloseOpID:
    case ReadOpID:
    case WriteOpID:
    case PositionOpID:
    case PInfOpID:
    case NInfOpID:
    case NaNOpID:
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
