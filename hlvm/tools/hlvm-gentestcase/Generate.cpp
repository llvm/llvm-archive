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

namespace 
{

cl::opt<unsigned>
  Complexity("complexity", 
    cl::init(5),
    cl::desc("Specify complexity of generated code"), 
    cl::value_desc("num"));

cl::opt<unsigned>
  TypeComplexity("type-complexity", 
    cl::init(4),
    cl::desc("Specify type complexity of generated code"), 
    cl::value_desc("num"));

cl::opt<unsigned>
  Seed("seed", 
    cl::init(unsigned(time(0))), 
    cl::desc("Specify random number generator seed"), 
    cl::value_desc("num"));

cl::opt<unsigned>
  Size("size",cl::desc("Specify size of generated code"),
      cl::value_desc("num"));

AST* ast = 0;
URI* uri = 0;
Bundle* bundle = 0;
Program* program = 0;
unsigned line = 0;
typedef std::vector<Value*> ValueList;
typedef std::map<const Type*,ValueList> TypeValueMap;
TypeValueMap values;
typedef std::vector<Type*> TypeList;
TypeList types;

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

Type*
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
          genType(limit),randRange(1,Size),loc);
      break;
    }
    case VectorTypeID:
    {
      Locator* loc = getLocator();
      std::string name = "vector_" + utostr(line);
      result = ast->new_VectorType(name,bundle,
          genType(limit),randRange(1,Size),loc);
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

Type*
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

Value*
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
      val += char(randRange(35,126));
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
      C = ast->new_ConstantString(
        std::string("cstr_")+utostr(line),bundle,Ty,val,loc);
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

inline Operator*
genValueOperator(const Type *Ty, bool is_constant = false)
{
  Value* V = genValue(Ty,is_constant);
  Operator* O = ast->new_ReferenceOp(V,getLocator());
  if (isa<Linkable>(V))
    O = ast->new_UnaryOp<LoadOp>(O,bundle,getLocator());
  return O;
}


CallOp*
genCallTo(Function* F)
{
  std::vector<Operator*> args;
  Operator* O = ast->new_ReferenceOp(F,getLocator());
  args.push_back(O);
  const SignatureType* sig = F->getSignature();
  for (SignatureType::const_iterator I = sig->begin(), E = sig->end(); 
       I != E; ++I) 
  {
    const Type* argTy = (*I)->getType();
    Operator* O = genValueOperator(argTy);
    hlvmAssert(argTy == O->getType());
    args.push_back(O);
  }
  return ast->new_MultiOp<CallOp>(args,bundle,getLocator());
}

Block*
genBlock()
{
  Block* B = ast->new_Block(getLocator());
  return B;
}

Function*
genFunction(Type* resultType, unsigned numArgs)
{
  // Get the function name
  Locator* loc = getLocator();
  std::string name = "func_" + utostr(line);

  // Get the signature
  std::string sigName = name + "_type";
  SignatureType* sig = ast->new_SignatureType(sigName,bundle,resultType,loc);
  if (randRange(0,Complexity) > int(Complexity/3))
    sig->setIsVarArgs(true);
  for (unsigned i = 0; i < numArgs; ++i )
  {
    std::string name = "arg_" + utostr(i+1);
    Parameter* param = ast->new_Parameter(name,genType(),loc);
    sig->addParameter(param);
  }
  sig->setParent(bundle);

  // Create the function and set its linkage
  LinkageKinds LK = LinkageKinds(randRange(ExternalLinkage,InternalLinkage));
  if (LK == AppendingLinkage)
    LK = InternalLinkage;
  Function* F = ast->new_Function(name,bundle,sig,loc);
  F->setLinkageKind(LK);

  // Create a block and set its parent
  Block* B = genBlock();
  B->setParent(F);

  // Get the function result and return instruction
  Operator* O = genValueOperator(F->getResultType());
  ResultOp* rslt = ast->new_UnaryOp<ResultOp>(O,bundle,getLocator());
  rslt->setParent(B);

  ReturnOp* ret = ast->new_NilaryOp<ReturnOp>(bundle,getLocator());
  ret->setParent(B);
  
  // Install the function in the value map
  values[sig].push_back(F);

  return F;
}

}

AST* 
GenerateTestCase(const std::string& pubid, const std::string& bundleName)
{
  srandom(Seed);
  ast = AST::create();
  ast->setPublicID(pubid);
  ast->setSystemID(bundleName);
  uri = ast->new_URI(pubid);
  bundle = ast->new_Bundle(bundleName,getLocator());
  program = ast->new_Program(bundleName,bundle,getLocator());
  Block* blk = ast->new_Block(getLocator());
  blk->setParent(program);
  for (unsigned i = 0; i < Size; i++) {
    Type* result = genType();
    Type* argTy  = genType();
    unsigned numArgs = int(randRange(0,int(Complexity)));
    Function* F = genFunction(result,numArgs);
    F->setParent(bundle);
    CallOp* call = genCallTo(F);
    call->setParent(blk);
  }

  // Get the function result and return instruction
  Operator* O = genValueOperator(program->getResultType());
  ResultOp* rslt = ast->new_UnaryOp<ResultOp>(O,bundle,getLocator());
  rslt->setParent(blk);

  ReturnOp* ret = ast->new_NilaryOp<ReturnOp>(bundle,getLocator());
  ret->setParent(blk);
  program->setParent(bundle);
  return ast;
}
