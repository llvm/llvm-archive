#include "llvm-abi.h"

SVR4ABI::SVR4ABI(DefaultABIClient &c) : C(c) {}

bool SVR4ABI::isShadowReturn() const { return C.isShadowReturn(); }

/// HandleReturnType - This is invoked by the target-independent code for the
/// return type. It potentially breaks down the argument and invokes methods
/// on the client that indicate how its pieces should be handled.  This
/// handles things like returning structures via hidden parameters.
void SVR4ABI::HandleReturnType(tree type, tree fn, bool isBuiltin) {
  unsigned Offset = 0;
  const Type *Ty = ConvertType(type);
  if (isa<VectorType>(Ty)) {
    // Vector handling is weird on x86.  In particular builtin and
    // non-builtin function of the same return types can use different
    // calling conventions.
    tree ScalarType = LLVM_SHOULD_RETURN_VECTOR_AS_SCALAR(type, isBuiltin);
    if (ScalarType)
      C.HandleAggregateResultAsScalar(ConvertType(ScalarType));
    else if (LLVM_SHOULD_RETURN_VECTOR_AS_SHADOW(type, isBuiltin))
      C.HandleScalarShadowResult(Ty->getPointerTo(), false);
    else
      C.HandleScalarResult(Ty);
  } else if (Ty->isSingleValueType() || Ty->isVoidTy()) {
    // Return scalar values normally.
    C.HandleScalarResult(Ty);
  } else if (doNotUseShadowReturn(type, fn, C.getCallingConv())) {
    tree SingleElt = LLVM_SHOULD_RETURN_SELT_STRUCT_AS_SCALAR(type);
    if (SingleElt && TYPE_SIZE(SingleElt) && 
	TREE_CODE(TYPE_SIZE(SingleElt)) == INTEGER_CST &&
	TREE_INT_CST_LOW(TYPE_SIZE_UNIT(type)) == 
	TREE_INT_CST_LOW(TYPE_SIZE_UNIT(SingleElt))) {
      C.HandleAggregateResultAsScalar(ConvertType(SingleElt));
    } else {
      // Otherwise return as an integer value large enough to hold the entire
      // aggregate.
      if (const Type *AggrTy = LLVM_AGGR_TYPE_FOR_STRUCT_RETURN(type,
                                  C.getCallingConv()))
	C.HandleAggregateResultAsAggregate(AggrTy);
      else if (const Type* ScalarTy = 
	       LLVM_SCALAR_TYPE_FOR_STRUCT_RETURN(type, &Offset))
	C.HandleAggregateResultAsScalar(ScalarTy, Offset);
      else {
	assert(0 && "Unable to determine how to return this aggregate!");
	abort();
      }
    }
  } else {
    // If the function is returning a struct or union, we pass the pointer to
    // the struct as the first argument to the function.

    // FIXME: should return the hidden first argument for some targets
    // (e.g. ELF i386).
    C.HandleAggregateShadowResult(Ty->getPointerTo(), false);
  }
}

static unsigned count_num_registers_uses(std::vector<const Type*> &ScalarElts) {
  unsigned NumGPRs = 0;
  for (unsigned i = 0, e = ScalarElts.size(); i != e; ++i) {
    if (NumGPRs >= 8)
      break;
    const Type *Ty = ScalarElts[i];
    if (const VectorType *VTy = dyn_cast<VectorType>(Ty)) {
      abort();
    } else if (isa<PointerType>(Ty)) {
      NumGPRs++;
    } else if (Ty->isInteger()) {
      unsigned TypeSize = Ty->getPrimitiveSizeInBits();
      unsigned NumRegs = (TypeSize + 31) / 32;

      NumGPRs += NumRegs;
    } else if (Ty->isVoidTy()) {
      // Padding bytes that are not passed anywhere
      ;
    } else {
      // Floating point scalar argument.
      assert(Ty->isFloatingPoint() && Ty->isPrimitiveType() &&
             "Expecting a floating point primitive type!");
    }
  }
  return NumGPRs < 8 ? NumGPRs : 8;
}

/// HandleArgument - This is invoked by the target-independent code for each
/// argument type passed into the function.  It potentially breaks down the
/// argument and invokes methods on the client that indicate how its pieces
/// should be handled.  This handles things like decimating structures into
/// their fields.
///
/// _Complex arguments are never split, thus their two scalars are either
/// passed both in argument registers or both on the stack. Also _Complex
/// arguments are always passed in general purpose registers, never in
/// Floating-point registers or vector registers.
void SVR4ABI::HandleArgument(tree type, std::vector<const Type*> &ScalarElts,
			     Attributes *Attributes) {
  unsigned Size = 0;
  bool DontCheckAlignment = false;
  // Eight GPR's are availabe for parameter passing.
  const unsigned NumArgRegs = 8;
  unsigned NumGPR = count_num_registers_uses(ScalarElts);
  const Type *Ty = ConvertType(type);
  // Figure out if this field is zero bits wide, e.g. {} or [0 x int].  Do
  // not include variable sized fields here.
  std::vector<const Type*> Elts;
  const Type* Int32Ty = Type::getInt32Ty(getGlobalContext());
  if (Ty->isVoidTy()) {
    // Handle void explicitly as an opaque type.
    const Type *OpTy = OpaqueType::get(getGlobalContext());
    C.HandleScalarArgument(OpTy, type);
    ScalarElts.push_back(OpTy);
  } else if (isPassedByInvisibleReference(type)) { // variable size -> by-ref.
    const Type *PtrTy = Ty->getPointerTo();
    C.HandleByInvisibleReferenceArgument(PtrTy, type);
    ScalarElts.push_back(PtrTy);
  } else if (isa<VectorType>(Ty)) {
    if (LLVM_SHOULD_PASS_VECTOR_IN_INTEGER_REGS(type)) {
      PassInIntegerRegisters(type, ScalarElts, 0, false);
    } else if (LLVM_SHOULD_PASS_VECTOR_USING_BYVAL_ATTR(type)) {
      C.HandleByValArgument(Ty, type);
      if (Attributes) {
	*Attributes |= Attribute::ByVal;
	*Attributes |= 
	  Attribute::constructAlignmentFromInt(LLVM_BYVAL_ALIGNMENT(type));
      }
    } else {
      C.HandleScalarArgument(Ty, type);
      ScalarElts.push_back(Ty);
    }
  } else if (Ty->isSingleValueType()) {
    if (Ty->isInteger()) {
      unsigned TypeSize = Ty->getPrimitiveSizeInBits();

      // Determine how many general purpose registers are needed for the
      // argument.
      unsigned NumRegs = (TypeSize + 31) / 32;

      // Make sure argument registers are aligned. 64-bit arguments are put in
      // a register pair which starts with an odd register number.
      if (TypeSize == 64 && (NumGPR % 2) == 1) {
	NumGPR++;
	ScalarElts.push_back(Int32Ty);
	C.HandlePad(Int32Ty);
      }

      if (NumGPR > (NumArgRegs - NumRegs)) {
	for (unsigned int i = 0; i < NumArgRegs - NumGPR; ++i) {
	  ScalarElts.push_back(Int32Ty);
	  C.HandlePad(Int32Ty);
	}
      }
    } else if (!(Ty->isFloatingPoint() ||
		 isa<VectorType>(Ty)   ||
		 isa<PointerType>(Ty))) {
      abort();
    }

    C.HandleScalarArgument(Ty, type);
    ScalarElts.push_back(Ty);
  } else if (LLVM_SHOULD_PASS_AGGREGATE_AS_FCA(type, Ty)) {
    C.HandleFCAArgument(Ty, type);
  } else if (LLVM_SHOULD_PASS_AGGREGATE_IN_MIXED_REGS(type, Ty,
						      C.getCallingConv(),
						      Elts)) {
    HOST_WIDE_INT SrcSize = int_size_in_bytes(type);

    // With the SVR4 ABI, the only aggregates which are passed in registers
    // are _Complex aggregates.
    assert(TREE_CODE(type) == COMPLEX_TYPE && "Not a _Complex type!");

    switch (SrcSize) {
    default:
      abort();
      break;
    case 32:
      // _Complex long double
      if (NumGPR != 0) {
	for (unsigned int i = 0; i < NumArgRegs - NumGPR; ++i) {
	  ScalarElts.push_back(Int32Ty);
	  C.HandlePad(Int32Ty);
	}
      }
      break;
    case 16:
      // _Complex long long
      // _Complex double
      if (NumGPR > (NumArgRegs - 4)) {
	for (unsigned int i = 0; i < NumArgRegs - NumGPR; ++i) {
	  ScalarElts.push_back(Int32Ty);
	  C.HandlePad(Int32Ty);
	}
      }
      break;
    case 8:
      // _Complex int
      // _Complex long
      // _Complex float

      // Make sure argument registers are aligned. 64-bit arguments are put in
      // a register pair which starts with an odd register number.
      if (NumGPR % 2 == 1) {
	NumGPR++;
	ScalarElts.push_back(Int32Ty);
	C.HandlePad(Int32Ty);
      }

      if (NumGPR > (NumArgRegs - 2)) {
	for (unsigned int i = 0; i < NumArgRegs - NumGPR; ++i) {
	  ScalarElts.push_back(Int32Ty);
	  C.HandlePad(Int32Ty);
	}
      }
      break;
    case 4:
    case 2:
      // _Complex short
      // _Complex char
      break;
    }

    PassInMixedRegisters(Ty, Elts, ScalarElts);
  } else if (LLVM_SHOULD_PASS_AGGREGATE_USING_BYVAL_ATTR(type, Ty)) {
    C.HandleByValArgument(Ty, type);
    if (Attributes) {
      *Attributes |= Attribute::ByVal;
      *Attributes |= 
	Attribute::constructAlignmentFromInt(LLVM_BYVAL_ALIGNMENT(type));
    }
  } else if (LLVM_SHOULD_PASS_AGGREGATE_IN_INTEGER_REGS(type, &Size,
							&DontCheckAlignment)) {
    PassInIntegerRegisters(type, ScalarElts, Size, DontCheckAlignment);
  } else if (isZeroSizedStructOrUnion(type)) {
    // Zero sized struct or union, just drop it!
    ;
  } else if (TREE_CODE(type) == RECORD_TYPE) {
    for (tree Field = TYPE_FIELDS(type); Field; Field = TREE_CHAIN(Field))
      if (TREE_CODE(Field) == FIELD_DECL) {
	const tree Ftype = getDeclaredType(Field);
	const Type *FTy = ConvertType(Ftype);
	unsigned FNo = GET_LLVM_FIELD_INDEX(Field);
	assert(FNo != ~0U && "Case not handled yet!");

	// Currently, a bvyal type inside a non-byval struct is a zero-length
	// object inside a bigger object on x86-64.  This type should be
	// skipped (but only when it is inside a bigger object).
	// (We know there currently are no other such cases active because
	// they would hit the assert in FunctionPrologArgumentConversion::
	// HandleByValArgument.)
	if (!LLVM_SHOULD_PASS_AGGREGATE_USING_BYVAL_ATTR(Ftype, FTy)) {
	  C.EnterField(FNo, Ty);
	  HandleArgument(getDeclaredType(Field), ScalarElts);
	  C.ExitField();
	}
      }
  } else if (TREE_CODE(type) == COMPLEX_TYPE) {
    C.EnterField(0, Ty);
    HandleArgument(TREE_TYPE(type), ScalarElts);
    C.ExitField();
    C.EnterField(1, Ty);
    HandleArgument(TREE_TYPE(type), ScalarElts);
    C.ExitField();
  } else if ((TREE_CODE(type) == UNION_TYPE) ||
	     (TREE_CODE(type) == QUAL_UNION_TYPE)) {
    HandleUnion(type, ScalarElts);
  } else if (TREE_CODE(type) == ARRAY_TYPE) {
    const ArrayType *ATy = cast<ArrayType>(Ty);
    for (unsigned i = 0, e = ATy->getNumElements(); i != e; ++i) {
      C.EnterField(i, Ty);
      HandleArgument(TREE_TYPE(type), ScalarElts);
      C.ExitField();
    }
  } else {
    assert(0 && "unknown aggregate type!");
    abort();
  }
}

/// HandleUnion - Handle a UNION_TYPE or QUAL_UNION_TYPE tree.
void SVR4ABI::HandleUnion(tree type, std::vector<const Type*> &ScalarElts) {
  if (TYPE_TRANSPARENT_UNION(type)) {
    tree Field = TYPE_FIELDS(type);
    assert(Field && "Transparent union must have some elements!");
    while (TREE_CODE(Field) != FIELD_DECL) {
      Field = TREE_CHAIN(Field);
      assert(Field && "Transparent union must have some elements!");
    }

    HandleArgument(TREE_TYPE(Field), ScalarElts);
  } else {
    // Unions pass the largest element.
    unsigned MaxSize = 0;
    tree MaxElt = 0;
    for (tree Field = TYPE_FIELDS(type); Field; Field = TREE_CHAIN(Field)) {
      if (TREE_CODE(Field) == FIELD_DECL) {
	// Skip fields that are known not to be present.
	if (TREE_CODE(type) == QUAL_UNION_TYPE &&
	    integer_zerop(DECL_QUALIFIER(Field)))
	  continue;

	tree SizeTree = TYPE_SIZE(TREE_TYPE(Field));
	unsigned Size = ((unsigned)TREE_INT_CST_LOW(SizeTree)+7)/8;
	if (Size > MaxSize) {
	  MaxSize = Size;
	  MaxElt = Field;
	}

	// Skip remaining fields if this one is known to be present.
	if (TREE_CODE(type) == QUAL_UNION_TYPE &&
	    integer_onep(DECL_QUALIFIER(Field)))
	  break;
      }
    }

    if (MaxElt)
      HandleArgument(TREE_TYPE(MaxElt), ScalarElts);
  }
}

/// PassInIntegerRegisters - Given an aggregate value that should be passed in
/// integer registers, convert it to a structure containing ints and pass all
/// of the struct elements in.  If Size is set we pass only that many bytes.
void SVR4ABI::PassInIntegerRegisters(tree type,
				     std::vector<const Type*> &ScalarElts,
				     unsigned origSize,
				     bool DontCheckAlignment) {
  unsigned Size;
  if (origSize)
    Size = origSize;
  else
    Size = TREE_INT_CST_LOW(TYPE_SIZE(type))/8;

  // FIXME: We should preserve all aggregate value alignment information.
  // Work around to preserve some aggregate value alignment information:
  // don't bitcast aggregate value to Int64 if its alignment is different
  // from Int64 alignment. ARM backend needs this.
  unsigned Align = TYPE_ALIGN(type)/8;
  unsigned Int64Align =
    getTargetData().getABITypeAlignment(Type::getInt64Ty(getGlobalContext()));
  bool UseInt64 = (DontCheckAlignment || Align >= Int64Align);

  unsigned ElementSize = UseInt64 ? 8:4;
  unsigned ArraySize = Size / ElementSize;

  // Put as much of the aggregate as possible into an array.
  const Type *ATy = NULL;
  const Type *ArrayElementType = NULL;
  if (ArraySize) {
    Size = Size % ElementSize;
    ArrayElementType = (UseInt64 ?
			Type::getInt64Ty(getGlobalContext()) :
			Type::getInt32Ty(getGlobalContext()));
    ATy = ArrayType::get(ArrayElementType, ArraySize);
  }

  // Pass any leftover bytes as a separate element following the array.
  unsigned LastEltRealSize = 0;
  const llvm::Type *LastEltTy = 0;
  if (Size > 4) {
    LastEltTy = Type::getInt64Ty(getGlobalContext());
  } else if (Size > 2) {
    LastEltTy = Type::getInt32Ty(getGlobalContext());
  } else if (Size > 1) {
    LastEltTy = Type::getInt16Ty(getGlobalContext());
  } else if (Size > 0) {
    LastEltTy = Type::getInt8Ty(getGlobalContext());
  }
  if (LastEltTy) {
    if (Size != getTargetData().getTypeAllocSize(LastEltTy))
      LastEltRealSize = Size;
  }

  std::vector<const Type*> Elts;
  if (ATy)
    Elts.push_back(ATy);
  if (LastEltTy)
    Elts.push_back(LastEltTy);
  const StructType *STy = StructType::get(getGlobalContext(), Elts, false);

  unsigned i = 0;
  if (ArraySize) {
    C.EnterField(0, STy);
    for (unsigned j = 0; j < ArraySize; ++j) {
      C.EnterField(j, ATy);
      C.HandleScalarArgument(ArrayElementType, 0);
      ScalarElts.push_back(ArrayElementType);
      C.ExitField();
    }
    C.ExitField();
    ++i;
  }
  if (LastEltTy) {
    C.EnterField(i, STy);
    C.HandleScalarArgument(LastEltTy, 0, LastEltRealSize);
    ScalarElts.push_back(LastEltTy);
    C.ExitField();
  }
}

/// PassInMixedRegisters - Given an aggregate value that should be passed in
/// mixed integer, floating point, and vector registers, convert it to a
/// structure containing the specified struct elements in.
void SVR4ABI::PassInMixedRegisters(const Type *Ty,
				   std::vector<const Type*> &OrigElts,
				   std::vector<const Type*> &ScalarElts) {
  // We use VoidTy in OrigElts to mean "this is a word in the aggregate
  // that occupies storage but has no useful information, and is not passed
  // anywhere".  Happens on x86-64.
  std::vector<const Type*> Elts(OrigElts);
  const Type* wordType = getTargetData().getPointerSize() == 4 ?
    Type::getInt32Ty(getGlobalContext()) : Type::getInt64Ty(getGlobalContext());
  for (unsigned i=0, e=Elts.size(); i!=e; ++i)
    if (OrigElts[i]->isVoidTy())
      Elts[i] = wordType;

  const StructType *STy = StructType::get(getGlobalContext(), Elts, false);

  unsigned Size = getTargetData().getTypeAllocSize(STy);
  const StructType *InSTy = dyn_cast<StructType>(Ty);
  unsigned InSize = 0;
  // If Ty and STy size does not match then last element is accessing
  // extra bits.
  unsigned LastEltSizeDiff = 0;
  if (InSTy) {
    InSize = getTargetData().getTypeAllocSize(InSTy);
    if (InSize < Size) {
      unsigned N = STy->getNumElements();
      const llvm::Type *LastEltTy = STy->getElementType(N-1);
      if (LastEltTy->isInteger())
	LastEltSizeDiff = 
	  getTargetData().getTypeAllocSize(LastEltTy) - (Size - InSize);
    }
  }
  for (unsigned i = 0, e = Elts.size(); i != e; ++i) {
    if (!OrigElts[i]->isVoidTy()) {
      C.EnterField(i, STy);
      unsigned RealSize = 0;
      if (LastEltSizeDiff && i == (e - 1))
	RealSize = LastEltSizeDiff;
      C.HandleScalarArgument(Elts[i], 0, RealSize);
      ScalarElts.push_back(Elts[i]);
      C.ExitField();
    }
  }
}
