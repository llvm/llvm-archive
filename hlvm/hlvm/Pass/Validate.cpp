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
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/RuntimeType.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/LinkageItems.h>
#include <hlvm/AST/Constants.h>
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
    inline void validateName(Node*n, const std::string& name);

    template <class NodeClass>
    inline void validate(NodeClass* C);
};

void 
ValidateImpl::error(Node* n, const char* msg)
{
  std::cerr << "Node(" << n << "): " << msg << "\n";
}

inline void
ValidateImpl::validateName(Node*n, const std::string& name)
{
  if (name.empty()) {
    error(0,"Empty Name");
  }
}

template<> inline void
ValidateImpl::validate(VoidType* n)
{
}

template<> inline void
ValidateImpl::validate(AnyType* n)
{
}

template<> inline void
ValidateImpl::validate(BooleanType* n)
{
}

template<> inline void
ValidateImpl::validate(CharacterType* n)
{
}

template<> inline void
ValidateImpl::validate(OctetType* n)
{
}

template<> inline void
ValidateImpl::validate(IntegerType* n)
{
  validateName(n,n->getName());
  if (n->getBits() == 0) {
    error(n,"Invalid number of bits");
  }
}

template<> inline void
ValidateImpl::validate(RangeType* n)
{
}

template<> inline void
ValidateImpl::validate(EnumerationType* n)
{
}

template<> inline void
ValidateImpl::validate(RealType* n)
{
}

template<> inline void
ValidateImpl::validate(OpaqueType* n)
{
}

template<> inline void
ValidateImpl::validate(TextType* n)
{
}

template<> inline void
ValidateImpl::validate(StreamType* n)
{
}

template<> inline void
ValidateImpl::validate(BufferType* n)
{
}

template<> inline void
ValidateImpl::validate(AliasType* n)
{
}

template<> inline void
ValidateImpl::validate(PointerType* n)
{
}

template<> inline void
ValidateImpl::validate(ArrayType* n)
{
}

template<> inline void
ValidateImpl::validate(VectorType* n)
{
}

template<> inline void
ValidateImpl::validate(StructureType* n)
{
}

template<> inline void
ValidateImpl::validate(ContinuationType* n)
{
}

template<> inline void
ValidateImpl::validate(SignatureType* n)
{
}

template<> inline void
ValidateImpl::validate(Variable* n)
{
}

template<> inline void
ValidateImpl::validate(Function* n)
{
}

template<> inline void
ValidateImpl::validate(Program* n)
{
}

template<> inline void
ValidateImpl::validate(Block* n)
{
}

template<> inline void
ValidateImpl::validate(ReturnOp* n)
{
}

template<> inline void
ValidateImpl::validate(BreakOp* n)
{
}

template<> inline void
ValidateImpl::validate(ContinueOp* n)
{
}

template<> inline void
ValidateImpl::validate(SelectOp* n)
{
}

template<> inline void
ValidateImpl::validate(LoopOp* n)
{
}

template<> inline void
ValidateImpl::validate(SwitchOp* n)
{
}

template<> inline void
ValidateImpl::validate(AllocateOp* n)
{
}

template<> inline void
ValidateImpl::validate(DeallocateOp* n)
{
}

template<> inline void
ValidateImpl::validate(LoadOp* n)
{
}

template<> inline void
ValidateImpl::validate(StoreOp* n)
{
}

template<> inline void
ValidateImpl::validate(AutoVarOp* n)
{
}

template<> inline void
ValidateImpl::validate(NegateOp* n)
{
}

template<> inline void
ValidateImpl::validate(ComplementOp* n)
{
}

template<> inline void
ValidateImpl::validate(PreIncrOp* n)
{
}

template<> inline void
ValidateImpl::validate(PostIncrOp* n)
{
}

template<> inline void
ValidateImpl::validate(PreDecrOp* n)
{
}

template<> inline void
ValidateImpl::validate(PostDecrOp* n)
{
}

template<> inline void
ValidateImpl::validate(AddOp* n)
{
}

template<> inline void
ValidateImpl::validate(SubtractOp* n)
{
}

template<> inline void
ValidateImpl::validate(MultiplyOp* n)
{
}

template<> inline void
ValidateImpl::validate(DivideOp* n)
{
}

template<> inline void
ValidateImpl::validate(ModuloOp* n)
{
}

template<> inline void
ValidateImpl::validate(BAndOp* n)
{
}

template<> inline void
ValidateImpl::validate(BOrOp* n)
{
}

template<> inline void
ValidateImpl::validate(BXorOp* n)
{
}

template<> inline void
ValidateImpl::validate(NotOp* n)
{
}

template<> inline void
ValidateImpl::validate(AndOp* n)
{
}

template<> inline void
ValidateImpl::validate(OrOp* n)
{
}

template<> inline void
ValidateImpl::validate(NorOp* n)
{
}

template<> inline void
ValidateImpl::validate(XorOp* n)
{
}

template<> inline void
ValidateImpl::validate(LessThanOp* n)
{
}

template<> inline void
ValidateImpl::validate(GreaterThanOp* n)
{
}

template<> inline void
ValidateImpl::validate(LessEqualOp* n)
{
}

template<> inline void
ValidateImpl::validate(GreaterEqualOp* n)
{
}

template<> inline void
ValidateImpl::validate(EqualityOp* n)
{
}

template<> inline void
ValidateImpl::validate(InequalityOp* n)
{
}

template<> inline void
ValidateImpl::validate(IsPInfOp* n)
{
}

template<> inline void
ValidateImpl::validate(IsNInfOp* n)
{
}

template<> inline void
ValidateImpl::validate(IsNanOp* n)
{
}

template<> inline void
ValidateImpl::validate(TruncOp* n)
{
}

template<> inline void
ValidateImpl::validate(RoundOp* n)
{
}

template<> inline void
ValidateImpl::validate(FloorOp* n)
{
}

template<> inline void
ValidateImpl::validate(CeilingOp* n)
{
}

template<> inline void
ValidateImpl::validate(LogEOp* n)
{
}

template<> inline void
ValidateImpl::validate(Log2Op* n)
{
}

template<> inline void
ValidateImpl::validate(Log10Op* n)
{
}

template<> inline void
ValidateImpl::validate(SquareRootOp* n)
{
}

template<> inline void
ValidateImpl::validate(CubeRootOp* n)
{
}

template<> inline void
ValidateImpl::validate(FactorialOp* n)
{
}

template<> inline void
ValidateImpl::validate(PowerOp* n)
{
}

template<> inline void
ValidateImpl::validate(RootOp* n)
{
}

template<> inline void
ValidateImpl::validate(GCDOp* n)
{
}

template<> inline void
ValidateImpl::validate(LCMOp* n)
{
}


template<> inline void
ValidateImpl::validate(OpenOp* n)
{
}

template<> inline void
ValidateImpl::validate(CloseOp* n)
{
}

template<> inline void
ValidateImpl::validate(ReadOp* n)
{
}

template<> inline void
ValidateImpl::validate(WriteOp* n)
{
}

template<> inline void
ValidateImpl::validate(ConstantInteger* n)
{
}

template<> inline void
ValidateImpl::validate(ConstantReal* n)
{
}

template<> inline void
ValidateImpl::validate(ConstantText* n)
{
}

template<> inline void
ValidateImpl::validate(ConstantZero* n)
{
}

template<> inline void
ValidateImpl::validate(ConstantAggregate* n)
{
}

template<> inline void
ValidateImpl::validate(ConstantExpression* n)
{
}

template<> inline void
ValidateImpl::validate(Bundle* n)
{
}

template<> inline void
ValidateImpl::validate(Import* n)
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
    case ReturnOpID:             validate(cast<ReturnOp>(n)); break;
    case ThrowOpID:              /*validate(cast<ThrowOp>(n));*/ break;
    case ContinueOpID:           /*validate(cast<ContinueOp>(n));*/ break;
    case BreakOpID:              validate(cast<BreakOp>(n)); break;
    case SelectOpID:             validate(cast<SelectOp>(n)); break;
    case LoopOpID:               validate(cast<LoopOp>(n)); break;
    case SwitchOpID:             validate(cast<SwitchOp>(n)); break;
    case LoadOpID:               validate(cast<LoadOp>(n)); break;
    case StoreOpID:              validate(cast<StoreOp>(n)); break;
    case AllocateOpID:           validate(cast<AllocateOp>(n)); break;
    case DeallocateOpID:         validate(cast<DeallocateOp>(n)); break;
    case ReallocateOpID:         /*validate(cast<ReallocateOp>(n));*/ break;
    case ReferenceOpID:          /*validate(cast<ReferenceOp>(n));*/ break;
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
    case AndOpID:                validate(cast<AndOp>(n)); break;
    case OrOpID:                 validate(cast<OrOp>(n)); break;
    case NorOpID:                validate(cast<NorOp>(n)); break;
    case XorOpID:                validate(cast<XorOp>(n)); break;
    case NotOpID:                validate(cast<NotOp>(n)); break;
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
    case PInfOpID:               /*validate(cast<PInfOp>(n));*/ break;
    case NInfOpID:               /*validate(cast<NInfoOp>(n));*/ break;
    case NaNOpID:                /*validate(cast<NaNOp>(n));*/ break;
    case ConstantIntegerID:      validate(cast<ConstantInteger>(n)); break;
    case ConstantRealID:         validate(cast<ConstantReal>(n)); break;
    case ConstantTextID:         validate(cast<ConstantText>(n)); break;
    case ConstantZeroID:         validate(cast<ConstantZero>(n)); break;
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
