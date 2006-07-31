//===-- Generate Random Test Cases ------------------------------*- C++ -*-===//
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
/// @file tools/hlvm-gentestcase/Generate.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the test case generator for hlvm-gentestcase
//===----------------------------------------------------------------------===//

#include <hlvm/Base/Assert.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/AST/Bundle.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/ADT/StringExtras.h>
#include <stdlib.h>
#include <time.h>

using namespace llvm;
using namespace hlvm;

static cl::opt<unsigned>
  Complexity("complexity", 
    cl::init(5),
    cl::desc("Specify complexity of generated code"), 
    cl::value_desc("num"));

static cl::opt<unsigned>
  TypeComplexity("type-complexity", 
    cl::init(4),
    cl::desc("Specify type complexity of generated code"), 
    cl::value_desc("num"));

static cl::opt<unsigned>
  Seed("seed", 
    cl::init(unsigned(time(0))), 
    cl::desc("Specify random number generator seed"), 
    cl::value_desc("num"));

static cl::opt<unsigned>
  Size("size",cl::desc("Specify size of generated code"),
      cl::value_desc("num"));

typedef std::vector<Value*> ValueList;
typedef std::map<const Type*,ValueList> TypeValueMap;
typedef std::vector<Type*> TypeList;

static AST* ast = 0;
static URI* uri = 0;
static Bundle* bundle = 0;
static Program* program = 0;
static unsigned line = 0;
static TypeValueMap values;
static TypeList types;

inline Locator* 
getLocator()
{
  return ast->new_Locator(uri,++line);
}

inline 
int64_t randRange(int64_t low, int64_t high)
{
  if (high > low)
    return int64_t(random()) % (high-low) + low;
  else if (low > high)
    return int64_t(random()) % (low-high) + high;
  else
    return low;
}

inline
uint64_t randRange(uint64_t low, uint64_t high, bool discriminate)
{
  if (high > low)
    return uint64_t(random()) % (high-low) + low;
  else if (low > high)
    return uint64_t(random()) % (low-high) + high;
  else
    return low;
}

static Type*
genType(unsigned limit)
{
  Type* result = 0;
  bool intrinsic_type = randRange(0,TypeComplexity) < TypeComplexity/3;
  if (--limit == 0)
    intrinsic_type = true;

  if (intrinsic_type) {
    IntrinsicTypes theType = IntrinsicTypes(
        randRange(FirstIntrinsicType,LastIntrinsicType));
    // FIXME: Don't allow things we can't code gen right now
    if (theType == u128Ty)
      theType = u64Ty;
    else if (theType == s128Ty)
      theType = s64Ty;
    else if (theType == f128Ty || theType == f96Ty || theType == f80Ty)
      theType = f64Ty;
    else if (theType == bufferTy || theType == streamTy || theType == textTy)
      theType = s32Ty;
    return bundle->getIntrinsicType(theType);
  }

  NodeIDs id = NodeIDs(randRange(FirstTypeID,LastTypeID));
  switch (id) {
    case BooleanTypeID: 
      result = bundle->getIntrinsicType(boolTy);
      break;
    case CharacterTypeID:
      result = bundle->getIntrinsicType(charTy);
      break;
    case StringTypeID:
      result = bundle->getIntrinsicType(stringTy);
      break;
    case AnyTypeID:
    case BufferTypeID:
    case StreamTypeID:
    case TextTypeID:
      /* FALL THROUGH (unimplemented) */
    case SignatureTypeID:
      /* FALL THROUGH (unimplemented) */
    case RationalTypeID:
      /* FALL THROUGH (unimplemented) */
    case IntegerTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "int_" + utostr(line);
      bool isSigned = randRange(0,TypeComplexity+2,true) < (TypeComplexity+2)/2;
      result = 
        ast->new_IntegerType(name,bundle,randRange(4,64,true),isSigned,loc);
      break;
    }
    case RangeTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "range_" + utostr(line);
      int64_t limit = randRange(0,8000000);
      result = ast->new_RangeType(name,bundle,-limit,limit,loc);
      break;
    }
    case EnumerationTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "enum_" + utostr(line);
      EnumerationType* E = ast->new_EnumerationType(name,bundle,loc);
      unsigned numEnums = randRange(1,TypeComplexity,true);
      for (unsigned i = 0; i < numEnums; i++)
        E->addEnumerator(name + "_" + utostr(i));
      result = E;
      break;
    }
    case RealTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "real_" + utostr(line);
      result = 
        ast->new_RealType(name,bundle,randRange(8,52),randRange(8,11),loc);
      break;
    }
    case PointerTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "ptr_" + utostr(line);
      result = ast->new_PointerType(name,bundle,genType(limit),loc);
      break;
    }
    case ArrayTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "array_" + utostr(line);
      result = ast->new_ArrayType(name,bundle,
          genType(limit),randRange(1,Size*10),loc);
      break;
    }
    case VectorTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "vector_" + utostr(line);
      result = ast->new_VectorType(name,bundle,
          genType(limit),randRange(1,Size*10),loc);
      break;
    }
    case OpaqueTypeID:
    case ContinuationTypeID:
      /* FALL THROUGH (not implemented) */
    case StructureTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "struct_" + utostr(line);
      StructureType* S = ast->new_StructureType(name,bundle,loc);
      unsigned numFields = randRange(1,Size,true);
      for (unsigned i = 0; i < numFields; ++i) {
        Field* fld = ast->new_Field(name+"_"+utostr(i),
            genType(limit),getLocator());
        S->addField(fld);
      }
      result = S;
      break;
    }
    default:
      hlvmAssert(!"Invalid Type?");
  }
  hlvmAssert(result && "No type defined?");
  return result;
}

static Type*
genType()
{
  bool shouldGenNewType = randRange(0,5) < TypeComplexity;
  if (types.empty() || shouldGenNewType) {
    Type* Ty = genType(TypeComplexity);
    types.push_back(Ty);
    return Ty;
  }
  return types[ randRange(0,types.size()-1) ];
}

static ConstantString*
getConstantString(const std::string& str)
{
  typedef std::map<std::string,ConstantString*> strmap;
  static strmap stringmap;
  strmap::iterator I = stringmap.find(str);
  if (I == stringmap.end()) {
    Locator* loc = getLocator();
    Type* Ty = bundle->getIntrinsicType(stringTy);
    ConstantString* cstr = ast->new_ConstantString(
      std::string("cstr_")+utostr(line),bundle,Ty,str,loc);
    stringmap[str] = cstr;
    return cstr;
  }
  return I->second;
}

static ConstantInteger*
getConstantInteger(int32_t val)
{
  typedef std::map<int32_t,ConstantInteger*> intmap;
  static intmap integermap;
  intmap::iterator I = integermap.find(val);
  if (I == integermap.end()) {
    Locator* loc = getLocator();
    Type* Ty = bundle->getIntrinsicType(intTy);
    std::string val_str = itostr(val);
    ConstantInteger* cint = ast->new_ConstantInteger(
      std::string("cint_")+utostr(line),bundle,Ty,val_str,10,loc);
    integermap[val] = cint;
    return cint;
  }
  return I->second;
}

static Value*
genValue(const Type* Ty, bool is_constant = false)
{
  if (!is_constant && randRange(0,Complexity) < Complexity/2) {
    // First look up an existing value in the map
    TypeValueMap::iterator VI = values.find(Ty);
    if (VI != values.end()) {
      ValueList& VL = VI->second;
      unsigned index = randRange(0,VL.size()-1,true);
      Value* result = VL[index];
      hlvmAssert(result->getType() == Ty);
      return result;
    }
  }

  // Didn't find one in the map, so generate a variable or constant
  ConstantValue* C = 0;
  Locator* loc = getLocator();
  NodeIDs id = Ty->getID();
  switch (id) {
    case BooleanTypeID:
    {
      bool val = randRange(0,Complexity+2) < (Complexity+2)/2;
      C = ast->new_ConstantBoolean(
          std::string("cbool_") + utostr(line), bundle,Ty, val, loc);
      break;
    }
    case CharacterTypeID:
    {
      std::string val;
      if ( randRange(0,20) < Complexity )  {
        static char hexDigits[16] = { 
          '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        val = '#';
        val += hexDigits[ randRange(0,15) ];
        val += hexDigits[ randRange(0,15) ];
        val += hexDigits[ randRange(0,15) ];
        val += hexDigits[ randRange(0,15) ];
      } else {
        val += char(randRange(36,126));
      }
      C = ast->new_ConstantCharacter(
        std::string("cchar_") + utostr(line), bundle,Ty, val, loc);
      break;
    }
    case StringTypeID:
    {
      std::string val;
      unsigned numChars = randRange(1,Size+Complexity,true);
      for (unsigned i = 0 ; i < numChars; i++)
        val += char(randRange(35,126));
      C = getConstantString(val);
      break;
    }
    case BufferTypeID:
    case StreamTypeID:
    case TextTypeID:
    case AnyTypeID:
      // hlvmAssert("Can't get constant for these types");
      /* FALL THROUGH (unimplemented) */
    case SignatureTypeID:
      /* FALL THROUGH (unimplemented) */
    case RationalTypeID:
      /* FALL THROUGH (unimplemented) */
    case IntegerTypeID:
    {
      std::string name = "cint_" + utostr(line);
      const IntegerType* IntTy = llvm::cast<IntegerType>(Ty);
      unsigned bits = (IntTy->getBits() < 63 ? IntTy->getBits() : 63) - 2;
      int64_t max = 1 << bits;
      std::string val_str;
      if (IntTy->isSigned()) {
        int64_t val = randRange(int64_t(-max),int64_t(max-1));
        val_str = itostr(val);
      } else {
        uint64_t val = randRange(uint64_t(0),uint64_t(max),true);
        val_str = utostr(val);
      }
      C = ast->new_ConstantInteger(name,bundle,Ty,val_str,10,loc);
      break;
    }
    case RangeTypeID:
    {
      std::string name = "crange_" + utostr(line);
      const RangeType* RngTy = llvm::cast<RangeType>(Ty);
      int64_t val = randRange(RngTy->getMin(),RngTy->getMax());
      std::string val_str( itostr(val) );
      C = ast->new_ConstantInteger(name,bundle,RngTy,val_str,10,loc);
      break;
    }
    case EnumerationTypeID:
    {
      std::string name = "cenum_" + utostr(line);
      const EnumerationType* ETy = llvm::cast<EnumerationType>(Ty);
      unsigned val = randRange(0,ETy->size()-1);
      EnumerationType::const_iterator I = ETy->begin() + val;
      C = ast->new_ConstantEnumerator(name,bundle,ETy,*I,loc);
      break;
    }
    case RealTypeID:
    {
      double val = double(randRange(-10000000,10000000));
      std::string val_str(ftostr(val));
      C = ast->new_ConstantReal(
        std::string("cf32_")+utostr(line),bundle,Ty,val_str,loc);
      break;
    }
    case PointerTypeID:
    {
      const PointerType* PT = llvm::cast<PointerType>(Ty);
      const Type* refType = PT->getElementType();
      std::string name = std::string("cptr_") + utostr(line);
      Value* refValue = genValue(refType,true);
      C = ast->new_ConstantPointer(name, bundle,PT, 
        cast<ConstantValue>(refValue),loc);
      break;
    }
    case ArrayTypeID:
    {
      const ArrayType* AT = llvm::cast<ArrayType>(Ty);
      const Type* elemTy = AT->getElementType();
      unsigned nElems = randRange(1,AT->getMaxSize(),true);
      std::vector<ConstantValue*> elems;
      std::string name = "cptr_" + utostr(line);
      for (unsigned i = 0; i < nElems; i++)
        elems.push_back(cast<ConstantValue>(genValue(elemTy,true)));
      C = ast->new_ConstantArray(name, bundle,AT,elems, loc);
      break;
    }
    case VectorTypeID:
    {
      const VectorType* VT = llvm::cast<VectorType>(Ty);
      const Type* elemTy = VT->getElementType();
      uint64_t nElems = VT->getSize();
      std::string name = "cvect_" + utostr(line);
      std::vector<ConstantValue*> elems;
      for (unsigned i = 0; i < nElems; i++)
        elems.push_back(cast<ConstantValue>(genValue(elemTy,true)));
      C = ast->new_ConstantVector(name, bundle, VT, elems, loc);
      break;
    }
    case OpaqueTypeID:
      /* FALL THROUGH (not implemented) */
    case ContinuationTypeID:
      /* FALL THROUGH (not implemented) */
    case StructureTypeID:
    {
      const StructureType* ST = llvm::cast<StructureType>(Ty);
      std::string name = "cstruct_" + utostr(line);
      std::vector<ConstantValue*> elems;
      for (StructureType::const_iterator I = ST->begin(), E = ST->end(); 
           I != E; ++I) {
        const Type* Ty = (*I)->getType();
        Value* V = genValue(Ty,true);
        elems.push_back(cast<ConstantValue>(V));
      }
      C = ast->new_ConstantStructure(name, bundle, ST, elems, loc);
      break;
    }
    default:
      hlvmAssert(!"Invalid Type?");
  }

  // Give the constant a home
  C->setParent(bundle);

  // Make it either an initialized variable or just the constant itself.
  Value* result = 0;
  if (is_constant || (randRange(0,Complexity+2) < (Complexity+2)/2))
    result = C;
  else {
    Variable* var = 
      ast->new_Variable(C->getName()+"_var",bundle,C->getType(),loc);
    var->setIsConstant(false);
    var->setInitializer(C);
    var->setParent(bundle);
    result = var;
  }

  // Memoize the result
  values[result->getType()].push_back(result);
  return result;
}

static inline GetOp*
getReference(const Value* val)
{
  hlvmAssert(isa<Linkable>(val) || isa<Constant>(val) || isa<AutoVarOp>(val));
  return ast->new_GetOp(val,getLocator());
}

static inline Operator*
genValueAsOperator(const Type *Ty, bool is_constant = false)
{
  Value* V = genValue(Ty,is_constant);
  Operator* O = getReference(V);
  if (isa<Linkable>(V))
    O = ast->new_UnaryOp<LoadOp>(O,bundle,getLocator());
  return O;
}

static CallOp*
genCallTo(Function* F)
{
  std::vector<Operator*> args;
  Operator* O = ast->new_GetOp(F,getLocator());
  args.push_back(O);
  const SignatureType* sig = F->getSignature();
  for (SignatureType::const_iterator I = sig->begin(), E = sig->end(); 
       I != E; ++I) 
  {
    const Type* argTy = (*I)->getType();
    Operator* O = genValueAsOperator(argTy);
    hlvmAssert(argTy == O->getType());
    args.push_back(O);
  }
  return ast->new_MultiOp<CallOp>(args,bundle,getLocator());
}

static Operator*
genIndex(Operator* V)
{
  if (V->typeis<StructureType>()) {
    const StructureType* Ty = cast<StructureType>(V->getType());
    const NamedType* fldname = Ty->getField(randRange(0,Ty->size()-1));
    Constant* cfield = getConstantString(fldname->getName());
    GetOp* field  = getReference(cfield);
    return ast->new_BinaryOp<GetFieldOp>(V,field,bundle,getLocator());
  } else if (V->typeis<ArrayType>()) {
    const ArrayType* Ty = cast<ArrayType>(V->getType());
    Constant* cindex = getConstantInteger(0); //FIXME: gen rand at runtime
    GetOp* index = getReference(cindex);
    return ast->new_BinaryOp<GetIndexOp>(V,index,bundle,getLocator());
  } else if (V->typeis<VectorType>()) {
    const VectorType* Ty = cast<VectorType>(V->getType());
    int64_t idx = randRange(0,Ty->getSize()-1);
    Constant* cindex = getConstantInteger(idx);
    GetOp* index = getReference(cindex);
    return ast->new_BinaryOp<GetIndexOp>(V,index,bundle,getLocator());
  } else if (V->typeis<StringType>()) {
    const StringType* Ty = cast<StringType>(V->getType());
    Constant* cindex = getConstantInteger(0); //FIXME: gen rand at runtime
    GetOp* index = getReference(cindex);
    return ast->new_BinaryOp<GetIndexOp>(V,index,bundle,getLocator());
  } else if (V->typeis<PointerType>()) {
    Constant* cindex = getConstantInteger(0);
    GetOp* index = getReference(cindex);
    return ast->new_BinaryOp<GetIndexOp>(V,index,bundle,getLocator());
  } else
    hlvmAssert(!"Can't index this type!");
  return 0;
}

static Operator*
genBooleanUnary(Operator* V1) 
{
  hlvmAssert(V1->getType()->getID() == BooleanTypeID);
  return ast->new_UnaryOp<NotOp>(V1,bundle,getLocator());
}

static Operator*
genBooleanBinary(Operator* V1, Operator* V2) 
{
  hlvmAssert(V1->getType()->getID() == BooleanTypeID);
  hlvmAssert(V2->getType()->getID() == BooleanTypeID);
  Operator* result = 0;
  NodeIDs id = NodeIDs(randRange(AndOpID,InequalityOpID));
  switch (id) {
    case AndOpID:
      result = ast->new_BinaryOp<AndOp>(V1,V2,bundle,getLocator());
      break;
    case OrOpID:
      result = ast->new_BinaryOp<OrOp>(V1,V2,bundle,getLocator());
      break;
    case NorOpID:
      result = ast->new_BinaryOp<NorOp>(V1,V2,bundle,getLocator());
      break;
    case XorOpID:
      result = ast->new_BinaryOp<XorOp>(V1,V2,bundle,getLocator());
      break;
    case LessThanOpID:
      result = ast->new_BinaryOp<LessThanOp>(V1,V2,bundle,getLocator());
      break;
    case GreaterThanOpID:
      result = ast->new_BinaryOp<GreaterThanOp>(V1,V2,bundle,getLocator());
      break;
    case LessEqualOpID:
      result = ast->new_BinaryOp<LessEqualOp>(V1,V2,bundle,getLocator());
      break;
    case GreaterEqualOpID:
      result = ast->new_BinaryOp<GreaterEqualOp>(V1,V2,bundle,getLocator());
      break;
    case EqualityOpID:
      result = ast->new_BinaryOp<EqualityOp>(V1,V2,bundle,getLocator());
      break;
    case InequalityOpID:
      result = ast->new_BinaryOp<InequalityOp>(V1,V2,bundle,getLocator());
      break;
    default:
      hlvmAssert(!"Invalid boolean op ID");
  }
  return result;
}

static Operator*
genCharacterUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == CharacterTypeID);
  return 0;
}

static Operator*
genCharacterBinary(Operator* V1, Operator* V2) 
{
  hlvmAssert(V1->getType()->getID() == CharacterTypeID);
  hlvmAssert(V2->getType()->getID() == CharacterTypeID);
  return 0;
}

static Operator*
genIntegerUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == IntegerTypeID);
  Operator* result = 0;
  NodeIDs id = NodeIDs(randRange(NegateOpID, PostDecrOpID));
  switch (id) {
    case NegateOpID:
      result = ast->new_UnaryOp<NegateOp>(V1,bundle,getLocator());
      break;
    case ComplementOpID:
      result = ast->new_UnaryOp<ComplementOp>(V1,bundle,getLocator());
      break;
    case PreIncrOpID:
      result = ast->new_UnaryOp<PreIncrOp>(V1,bundle,getLocator());
      break;
    case PostIncrOpID:
      result = ast->new_UnaryOp<PostIncrOp>(V1,bundle,getLocator());
      break;
    case PreDecrOpID:
      result = ast->new_UnaryOp<PreDecrOp>(V1,bundle,getLocator());
      break;
    case PostDecrOpID:
      result = ast->new_UnaryOp<NegateOp>(V1,bundle,getLocator());
      break;
    default:
      hlvmAssert(!"Invalid unary op id for integer");
      result = ast->new_UnaryOp<ComplementOp>(V1,bundle,getLocator());
  }
  return result;
}

static Operator*
genIntegerBinary(Operator* V1, Operator* V2)
{
  hlvmAssert(V1->getType()->getID() == IntegerTypeID);
  hlvmAssert(V2->getType()->getID() == IntegerTypeID);
  Operator* result = 0;
  NodeIDs id = NodeIDs(randRange(AddOpID, BNorOpID));
  switch (id) {
    case AddOpID:
      result = ast->new_BinaryOp<AddOp>(V1,V2,bundle,getLocator());
      break;
    case SubtractOpID:
      result = ast->new_BinaryOp<SubtractOp>(V1,V2,bundle,getLocator());
      break;
    case MultiplyOpID:
      result = ast->new_BinaryOp<MultiplyOp>(V1,V2,bundle,getLocator());
      break;
    case DivideOpID:
      result = ast->new_BinaryOp<DivideOp>(V1,V2,bundle,getLocator());
      break;
    case ModuloOpID:
      result = ast->new_BinaryOp<ModuloOp>(V1,V2,bundle,getLocator());
      break;
    case BAndOpID:
      result = ast->new_BinaryOp<BAndOp>(V1,V2,bundle,getLocator());
      break;
    case BOrOpID:
      result = ast->new_BinaryOp<BOrOp>(V1,V2,bundle,getLocator());
      break;
    case BXorOpID:
      result = ast->new_BinaryOp<BXorOp>(V1,V2,bundle,getLocator());
      break;
    case BNorOpID:
      result = ast->new_BinaryOp<BNorOp>(V1,V2,bundle,getLocator());
      break;
    default:
      hlvmAssert(!"Invalid binary op id for integer");
      result = ast->new_BinaryOp<AddOp>(V1,V2,bundle,getLocator());
  }
  return result;
}

static Operator*
genRealUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  Operator* result = 0;
  NodeIDs id = NodeIDs(randRange(TruncOpID,FactorialOpID));
  switch (id) {
    case TruncOpID:
      result = ast->new_UnaryOp<TruncOp>(V1,bundle,getLocator());
      break;
    case RoundOpID:
      result = ast->new_UnaryOp<RoundOp>(V1,bundle,getLocator());
      break;
    case FloorOpID:
      result = ast->new_UnaryOp<FloorOp>(V1,bundle,getLocator());
      break;
    case CeilingOpID:
      result = ast->new_UnaryOp<CeilingOp>(V1,bundle,getLocator());
      break;
    case LogEOpID:
      result = ast->new_UnaryOp<LogEOp>(V1,bundle,getLocator());
      break;
    case Log2OpID:
      result = ast->new_UnaryOp<Log2Op>(V1,bundle,getLocator());
      break;
    case Log10OpID:
      result = ast->new_UnaryOp<Log10Op>(V1,bundle,getLocator());
      break;
    case SquareRootOpID:
      result = ast->new_UnaryOp<SquareRootOp>(V1,bundle,getLocator());
      break;
    case CubeRootOpID:
      result = ast->new_UnaryOp<CubeRootOp>(V1,bundle,getLocator());
      break;
    case FactorialOpID:
      result = ast->new_UnaryOp<FactorialOp>(V1,bundle,getLocator());
      break;
    default:
      hlvmAssert(!"Invalid unary op id for integer");
      result = ast->new_UnaryOp<CeilingOp>(V1,bundle,getLocator());
  }
  return result;
}

static Operator*
genRealBinary(Operator* V1, Operator* V2)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  hlvmAssert(V2->getType()->getID() == RealTypeID);
  Operator* result = 0;
  NodeIDs id = hlvm::NodeIDs(randRange(PowerOpID, LCMOpID));
  switch (id) {
    case PowerOpID:
      result = ast->new_BinaryOp<PowerOp>(V1,V2,bundle,getLocator());
      break;
    case RootOpID:
      result = ast->new_BinaryOp<RootOp>(V1,V2,bundle,getLocator());
      break;
    case GCDOpID:
      result = ast->new_BinaryOp<GCDOp>(V1,V2,bundle,getLocator());
      break;
    case LCMOpID:
      result = ast->new_BinaryOp<LCMOp>(V1,V2,bundle,getLocator());
      break;
    default:
      hlvmAssert(!"Invalid binary op id for integer");
      result = ast->new_BinaryOp<PowerOp>(V1,V2,bundle,getLocator());
  }
  return result;
}

static Operator*
genStringUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  return 0;
}

static Operator*
genStringBinary(Operator* V1, Operator* V2)
{
  return 0;
}

static Operator*
genPointerUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  return 0;
}

static Operator*
genPointerBinary(Operator* V1, Operator* V2)
{
  return 0;
}

static Operator*
genArrayUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  return 0;
}

static Operator*
genArrayBinary(Operator* V1, Operator* V2)
{
  return 0;
}

static Operator*
genVectorUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  return 0;
}

static Operator*
genVectorBinary(Operator* V1, Operator* V2)
{
  return 0;
}

static Operator*
genStructureUnary(Operator* V1)
{
  hlvmAssert(V1->getType()->getID() == RealTypeID);
  return 0;
}

static Operator*
genStructureBinary(Operator* V1, Operator* V2)
{
  return 0;
}

// Forward declare
static Function* genFunction(const Type* resultType, unsigned depth);

static Operator*
genExpression(Operator* Val, const Type* Ty, unsigned depth)
{
  hlvmAssert(Val->getType() == Ty);
  if (depth > 0) {
    // Generate a function
    Function* F = genFunction(Ty, depth-1);
    // Generate a call to that function
    return genCallTo(F);
  }

  Operator* result = 0;

  // Determine whether to generate a binary or unary expression
  if (5 <= randRange(1,10)) {
    switch (Ty->getID()) {
      case BooleanTypeID: result = genBooleanUnary(Val); break;
      case CharacterTypeID: result = genCharacterUnary(Val); break;
      case AnyTypeID:         
      case BufferTypeID:
      case StreamTypeID:
      case TextTypeID:
      case SignatureTypeID:
      case RationalTypeID:
        // FALL THROUGH: Not Implemented
      case RangeTypeID:
      case IntegerTypeID:
      case EnumerationTypeID: result = genIntegerUnary(Val); break;
      case RealTypeID: result = genRealUnary(Val); break;
      case StringTypeID: result = genStringUnary(Val); break;
      case PointerTypeID: result = genPointerUnary(Val); break;
      case ArrayTypeID: result = genArrayUnary(Val); break;
      case VectorTypeID: result = genVectorUnary(Val); break;
      case OpaqueTypeID:
      case ContinuationTypeID:
      case StructureTypeID: result = genStructureUnary(Val); break;
      default:
        hlvmAssert(!"Invalid type?");
        break;
    }
  } else {
    switch (Ty->getID()) {
      case BooleanTypeID: 
        result = genBooleanBinary(Val,genValueAsOperator(Ty)); break;
      case CharacterTypeID:
        result = genCharacterBinary(Val,genValueAsOperator(Ty)); break;
      case AnyTypeID:         
      case BufferTypeID:
      case StreamTypeID:
      case TextTypeID:
      case SignatureTypeID:
      case RationalTypeID:
        // FALL THROUGH: Not Implemented
      case RangeTypeID:
      case IntegerTypeID:
      case EnumerationTypeID: 
        result = genIntegerBinary(Val,genValueAsOperator(Ty));break;
      case RealTypeID: 
        result = genRealBinary(Val,genValueAsOperator(Ty)); break;
      case StringTypeID: 
        result = genStringBinary(Val,genValueAsOperator(Ty)); break;
      case PointerTypeID: 
        result = genPointerBinary(Val,genValueAsOperator(Ty)); break;
      case ArrayTypeID: 
        result = genArrayBinary(Val,genValueAsOperator(Ty)); break;
      case VectorTypeID: 
        result = genVectorBinary(Val,genValueAsOperator(Ty)); break;
      case OpaqueTypeID:
      case ContinuationTypeID:
      case StructureTypeID: 
        result = genStructureBinary(Val,genValueAsOperator(Ty));break;
      default:
        hlvmAssert(!"Invalid type?");
        break;
    }
  }
  return result;
}

static void
genMergeExpression(AutoVarOp* op1, Operator* op2, Block* B)
{
  // Assert precondition
  hlvmAssert(op1->getType() == op2->getType());

  // Get the type of the thing to be merged
  const Type* Ty = op2->getType();


  // If its just a numeric type, simply add the merged value into the autovar
  if (Ty->isNumericType()) {
    Operator* get1 = ast->new_GetOp(op1,getLocator());
    Operator* avload = ast->new_UnaryOp<LoadOp>(get1,bundle,getLocator());
    Operator* add = ast->new_BinaryOp<AddOp>(avload,op2,bundle,getLocator());
    Operator* get2 = ast->new_GetOp(op1,getLocator());
    Operator* store = ast->new_BinaryOp<StoreOp>(get2,add,bundle,getLocator());
    store->setParent(B);
    return;
  }

  // If its a VectorType or ArrayType then add all the elements, if possible
  if (const UniformContainerType* UCT = 
        llvm::dyn_cast<UniformContainerType>(op2->getType())) {
    const Type* elemType = UCT->getElementType();
    if (elemType->isNumericType()) {
      Operator* get1 = ast->new_GetOp(op1,getLocator());
      Operator* avload = ast->new_UnaryOp<LoadOp>(get1,bundle,getLocator());
      Operator* add = ast->new_BinaryOp<AddOp>(avload,op2,bundle,getLocator());
      Operator* get2 = ast->new_GetOp(op1,getLocator());
      Operator* store =ast->new_BinaryOp<StoreOp>(get2,add,bundle,getLocator());
      store->setParent(B);
      return;
    } 
  }

  // For everything else, just store the result
  Operator* get = ast->new_GetOp(op1,getLocator());
  Operator* op = ast->new_BinaryOp<StoreOp>(get,op2,bundle,getLocator());
  op->setParent(B);
  return;
}

static Block*
genFunctionBody(Function* F, unsigned depth)
{
  // Create the function body block and initialize it
  Block* B = ast->new_Block(getLocator());
  B->setParent(F);
  B->setLabel(F->getName() + "_body");

  // Create an autovar for the function result
  AutoVarOp* result = 
    ast->new_AutoVarOp(F->getName() + "_result",
                       F->getResultType(),getLocator());
  result->setParent(B);

  // Generate "Size" expressions
  for (unsigned i = 0; i < Size; i++) {
    // Get a value for the basis of the expression
    Operator* theValue = 0;
    if (F->size() > 0) {
      // Pick an argument randomly to use in the expression
      unsigned argNum = randRange(0,F->size()-1);
      Argument* arg = F->getArgument(argNum);
      theValue = ast->new_GetOp(arg,getLocator());
    } else {
      theValue = genValueAsOperator(F->getResultType());
    }

    // Generate the expression
    Operator* expr = genExpression(theValue,F->getResultType(),depth-1);

    // Merge the current value of the autovar result with the generated
    // expression.
    genMergeExpression(result,expr,B);
  }
  
  // Create the block result.
  GetOp* get = ast->new_GetOp(result,getLocator());
  ResultOp* rslt = ast->new_UnaryOp<ResultOp>(get,bundle,getLocator());
  rslt->setParent(B);

  // Done, Return the new block
  return B;
}

static Function*
genFunction(const Type* resultType, unsigned depth)
{
  // Get the function name
  Locator* loc = getLocator();
  std::string name = "func_" + utostr(line);

  // Generate a random number of arguments
  unsigned numArgs = int(randRange(0,int(Complexity)));

  // Get the signature
  std::string sigName = name + "_type";
  SignatureType* sig = ast->new_SignatureType(sigName,bundle,resultType,loc);
  if (randRange(0,Complexity) > int(Complexity/3))
    sig->setIsVarArgs(true);
  for (unsigned i = 0; i < numArgs; ++i )
  {
    std::string name = "arg_" + utostr(i+1);
    Parameter* param = ast->new_Parameter(name,resultType,loc);
    sig->addParameter(param);
  }
  sig->setParent(bundle);

  // Determine the kind of linkage for this function
  LinkageKinds LK = LinkageKinds(randRange(ExternalLinkage,InternalLinkage));
  if (LK == AppendingLinkage)
    LK = InternalLinkage;

  // Create the function and set its linkage kind
  Function* F = ast->new_Function(name,bundle,sig,loc);
  F->setLinkageKind(LK);

  // Create the body of the function
  Block* blk = genFunctionBody(F,depth);

  // Insert the return operator
  ReturnOp* ret = ast->new_NilaryOp<ReturnOp>(bundle,getLocator());
  ret->setParent(blk);
  
  // Install the function in the value map
  values[sig].push_back(F);

  // Make the function belong to the bundle
  F->setParent(bundle);

  return F;
}

AST* 
GenerateTestCase(const std::string& pubid, const std::string& bundleName)
{
  // Seed the random number generator
  srandom(Seed);

  // Create the top level node of the tree
  ast = AST::create();
  ast->setPublicID(pubid);
  ast->setSystemID(bundleName);

  // Get a URI for this tree
  uri = ast->new_URI(pubid);
  
  // Create a bundle to place the program in.
  bundle = ast->new_Bundle(bundleName,getLocator());

  // Create a program node for the generated test program
  program = ast->new_Program(bundleName,bundle,getLocator());
  const SignatureType* SigTy = program->getSignature();

  // Create the function body block and initialize it
  Block* B = ast->new_Block(getLocator());
  B->setParent(program);
  B->setLabel(program->getName() + "_body");

  // Create an autovar for the function result
  AutoVarOp* result = 
    ast->new_AutoVarOp(program->getName() + "_result",
                       program->getResultType(),getLocator());
  result->setParent(B);

  // Generate calls to "Size" random functions
  for (unsigned i = 0; i < Size; i++) {
    // Generate a function to a random type
    Type* Ty = genType();
    Function* F = genFunction(Ty, Complexity);

    // Generate a call to that function
    CallOp* call = genCallTo(F);

    // Coalesce the result of the random function into the autovar we
    // created for the result of this block.
    if (Ty->isNumericType()) {
      Operator* get = 
        ast->new_GetOp(program->getResultType(),getLocator());
      Operator* cvt = 
        ast->new_BinaryOp<ConvertOp>(call,get,bundle,getLocator());
      genMergeExpression(result,cvt,B);
    }
  }
  
  // Create the block result.
  GetOp* get = ast->new_GetOp(result,getLocator());
  ResultOp* rslt = ast->new_UnaryOp<ResultOp>(get,bundle,getLocator());
  rslt->setParent(B);

  // Insert the return instruction in that block
  ReturnOp* ret = ast->new_NilaryOp<ReturnOp>(bundle,getLocator());
  ret->setParent(B);

  // Make the program a child of the bundle last so it is output last.
  program->setParent(bundle);

  // Done.
  return ast;
}
