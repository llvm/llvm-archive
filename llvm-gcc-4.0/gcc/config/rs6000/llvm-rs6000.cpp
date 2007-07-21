/* APPLE LOCAL begin LLVM (ENTIRE FILE!)  */
/* High-level LLVM backend interface 
Copyright (C) 2007 Free Software Foundation, Inc.
Contributed by Jim Laskey (jlaskey@apple.com)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

//===----------------------------------------------------------------------===//
// This is a C++ source file that implements specific llvm powerpc ABI.
//===----------------------------------------------------------------------===//

#include "llvm-abi.h"
#include "llvm-internal.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/Module.h"

/* MergeIntPtrOperand - This merges the int and pointer operands of a GCC
 * intrinsic into a single operand for the LLVM intrinsic.  For example, this
 * turns LVX(4, p) -> llvm.lvx(gep P, 4).  OPNUM specifies the operand number
 * of the integer to contract with its following pointer and NAME specifies the
 * name of the resultant intrinsic.
 */
static void MergeIntPtrOperand(TreeToLLVM *TTL, Constant *&Cache,
                               unsigned OpNum, const char *Name,
                               const Type *ResultType,
                               std::vector<Value*> &Ops,
                               BasicBlock *CurBB, Value *&Result) {
  const Type *VoidPtrTy = PointerType::get(Type::Int8Ty);
  
  if (!Cache) {
    std::vector<const Type*> ArgTys;
    for (unsigned i = 0, e = Ops.size(); i != e; ++i)
      ArgTys.push_back(Ops[i]->getType());
      
    ArgTys.erase(ArgTys.begin() + OpNum);
    ArgTys[OpNum] = VoidPtrTy;
    FunctionType *FT = FunctionType::get(ResultType, ArgTys, false);
    Module *M = CurBB->getParent()->getParent();
    Cache = M->getOrInsertFunction(Name, FT);
  }
  
  Value *Offset = Ops[OpNum];
  Value *Ptr = Ops[OpNum + 1];
  Ptr = TTL->CastToType(Instruction::BitCast, Ptr, VoidPtrTy);
  
  if (!isa<Constant>(Offset) || !cast<Constant>(Offset)->isNullValue())
    Ptr = new GetElementPtrInst(Ptr, Offset, "tmp", CurBB);
    
  Ops.erase(Ops.begin() + OpNum);
  Ops[OpNum] = Ptr;
  Value *V = new CallInst(Cache, &Ops[0], Ops.size(), "", CurBB);
  
  if (V->getType() != Type::VoidTy) {
    V->setName("tmp");
    Result = V;
  }
}

/* GetAltivecTypeNumFromType - Given an LLVM type, return a unique ID for
 * the type in the range 0-3.
 */
static int GetAltivecTypeNumFromType(const Type *Ty) {
  return ((Ty == Type::Int32Ty) ? 0 : \
          ((Ty == Type::Int16Ty) ? 1 : \
           ((Ty == Type::Int8Ty) ? 2 : \
            ((Ty == Type::FloatTy) ? 3 : -1))));
}

/* GetAltivecLetterFromType - Given an LLVM type, return the altivec letter
 * for the type, e.g. int -> w.
 */
static char GetAltivecLetterFromType(const Type *Ty) {
  return ((Ty == Type::Int32Ty) ? 'w' :
          ((Ty == Type::Int16Ty) ? 'h' :
           ((Ty == Type::Int8Ty) ? 'b' :
            ((Ty == Type::FloatTy) ? 'f' : 'x'))));
}

/* TargetIntrinsicCastResult - This function just provides a frequently
 * used sequence for use inside TargetIntrinsicLower.
 */
static void TargetIntrinsicCastResult(Value *&Result, const Type *ResultType,
                                      bool ResIsSigned, bool ExpIsSigned,
                                      BasicBlock *CurBB) {
  Instruction::CastOps opcode =
    CastInst::getCastOpcode(Result, ResIsSigned, ResultType, ExpIsSigned);
  Result = CastInst::create(opcode, Result, ResultType, "tmp", CurBB);
}

/* IntrinsicOpIsSigned - This function determines if a given operand
 * to the intrinsic is signed or not.
 */
static bool IntrinsicOpIsSigned(SmallVector<tree, 8> &Args, unsigned OpNum) {
  return !TYPE_UNSIGNED(TREE_TYPE(Args[OpNum]));
}

/* TargetIntrinsicLower - To handle builtins, we want to expand the
 * invocation into normal LLVM code.  If the target can handle the builtin, this
 * function should emit the expanded code and return true.
 */
bool TreeToLLVM::TargetIntrinsicLower(unsigned FnCode,
                                      Value *DestLoc,
                                      Value *&Result,
                                      const Type *ResultType,
                                      std::vector<Value*> &Ops,
                                      SmallVector<tree, 8> &Args,
                                      BasicBlock *CurBB,
                                      bool ResIsSigned,
                                      bool ExpIsSigned) {
  switch (FnCode) {
  default: break;
  case ALTIVEC_BUILTIN_VADDFP:
  case ALTIVEC_BUILTIN_VADDUBM:
  case ALTIVEC_BUILTIN_VADDUHM:
  case ALTIVEC_BUILTIN_VADDUWM:
    Result = BinaryOperator::createAdd(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_VSUBFP:
  case ALTIVEC_BUILTIN_VSUBUBM:
  case ALTIVEC_BUILTIN_VSUBUHM:
  case ALTIVEC_BUILTIN_VSUBUWM:
    Result = BinaryOperator::createSub(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_VAND:
    Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_VANDC:
    Ops[1] = BinaryOperator::createNot(Ops[1], "tmp", CurBB);
    Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_VOR:
    Result = BinaryOperator::createOr(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_VNOR:
    Result = BinaryOperator::createOr(Ops[0], Ops[1], "tmp", CurBB);
    Result = BinaryOperator::createNot(Result, "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_VXOR:
    Result = BinaryOperator::createXor(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case ALTIVEC_BUILTIN_LVSL: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvsl",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_LVSR: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvsr",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_LVX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_LVXL: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvxl",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_LVEBX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvebx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_LVEHX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvehx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_LVEWX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 0, "llvm.ppc.altivec.lvewx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_STVX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 1, "llvm.ppc.altivec.stvx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_STVEBX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 1, "llvm.ppc.altivec.stvebx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_STVEHX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 1, "llvm.ppc.altivec.stvehx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_STVEWX: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 1, "llvm.ppc.altivec.stvewx",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_STVXL: {
      static Constant *Cache = NULL;
      MergeIntPtrOperand(this, Cache, 1, "llvm.ppc.altivec.stvxl",
                         ResultType, Ops, CurBB, Result);
    }
    return true;
  case ALTIVEC_BUILTIN_VSPLTISB:
    if (Constant *Elt = dyn_cast<ConstantInt>(Ops[0])) {
      Elt = ConstantExpr::getIntegerCast(Elt, Type::Int8Ty, true);
      Result = BuildVector(Elt, Elt, Elt, Elt,  Elt, Elt, Elt, Elt,
                           Elt, Elt, Elt, Elt,  Elt, Elt, Elt, Elt, NULL);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VSPLTISH:
    if (Constant *Elt = dyn_cast<ConstantInt>(Ops[0])) {
      Elt = ConstantExpr::getIntegerCast(Elt, Type::Int16Ty, true);
      Result = BuildVector(Elt, Elt, Elt, Elt,  Elt, Elt, Elt, Elt, NULL);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VSPLTISW:
    if (Constant *Elt = dyn_cast<ConstantInt>(Ops[0])) {
      Elt = ConstantExpr::getIntegerCast(Elt, Type::Int32Ty, true);
      Result = BuildVector(Elt, Elt, Elt, Elt, NULL);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VSPLTB:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[1])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[0],
                                  EV, EV, EV, EV, EV, EV, EV, EV,
                                  EV, EV, EV, EV, EV, EV, EV, EV);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VSPLTH:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[1])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[0],
                                  EV, EV, EV, EV, EV, EV, EV, EV);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VSPLTW:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[1])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[0], EV, EV, EV, EV);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VSLDOI_16QI:
  case ALTIVEC_BUILTIN_VSLDOI_8HI:
  case ALTIVEC_BUILTIN_VSLDOI_4SI:
  case ALTIVEC_BUILTIN_VSLDOI_4SF:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[2])) {
      /* Map all of these to a shuffle. */
      unsigned Amt = Elt->getZExtValue() & 15;
      PackedType *v16i8 = PackedType::get(Type::Int8Ty, 16);
      Value *Op0 = Ops[0];
      Instruction::CastOps opc = CastInst::getCastOpcode(Op0,
        IntrinsicOpIsSigned(Args,0), ResultType, false);
      Ops[0] = CastToType(opc, Op0, v16i8);
      Value *Op1 = Ops[1];
      opc = CastInst::getCastOpcode(Op1,
        IntrinsicOpIsSigned(Args,1), ResultType, false);
      Ops[1] = CastToType(opc, Op1, v16i8);
      Result = BuildVectorShuffle(Ops[0], Ops[1],
                                  Amt, Amt+1, Amt+2, Amt+3,
                                  Amt+4, Amt+5, Amt+6, Amt+7,
                                  Amt+8, Amt+9, Amt+10, Amt+11,
                                  Amt+12, Amt+13, Amt+14, Amt+15);
      return true;
    }
    return false;
  case ALTIVEC_BUILTIN_VPKUHUM: {
    Value *Op0 = Ops[0];
    Instruction::CastOps opc = CastInst::getCastOpcode(Op0,
        IntrinsicOpIsSigned(Args,0), ResultType, ExpIsSigned);
    Ops[0] = CastInst::create(opc, Op0, ResultType, Op0->getName(), CurBB);
    Value *Op1 = Ops[1];
    opc = CastInst::getCastOpcode(Op1,
        IntrinsicOpIsSigned(Args,1), ResultType, ExpIsSigned);
    Ops[1] = CastInst::create(opc, Op1, ResultType, Op1->getName(), CurBB);
    Result = BuildVectorShuffle(Ops[0], Ops[1], 1, 3, 5, 7, 9, 11, 13, 15,
                                17, 19, 21, 23, 25, 27, 29, 31);
    return true;
  }
  case ALTIVEC_BUILTIN_VPKUWUM: {
    Value *Op0 = Ops[0];
    Instruction::CastOps opc = CastInst::getCastOpcode(Op0,
        IntrinsicOpIsSigned(Args,0), ResultType, ExpIsSigned);
    Ops[0] = CastInst::create(opc, Op0, ResultType, Op0->getName(), CurBB);
    Value *Op1 = Ops[1];
    opc = CastInst::getCastOpcode(Op1,
        IntrinsicOpIsSigned(Args,1), ResultType, ExpIsSigned);
    Ops[1] = CastInst::create(opc, Op1, ResultType, Op1->getName(), CurBB);
    Result = BuildVectorShuffle(Ops[0], Ops[1], 1, 3, 5, 7, 9, 11, 13, 15);
    return true;
  }
  case ALTIVEC_BUILTIN_VMRGHB:
    Result = BuildVectorShuffle(Ops[0], Ops[1],
                                0, 16, 1, 17, 2, 18, 3, 19,
                                4, 20, 5, 21, 6, 22, 7, 23);
    return true;
  case ALTIVEC_BUILTIN_VMRGHH:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 8, 1, 9, 2, 10, 3, 11);
    return true;
  case ALTIVEC_BUILTIN_VMRGHW:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 4, 1, 5);
    return true;
  case ALTIVEC_BUILTIN_VMRGLB:
    Result = BuildVectorShuffle(Ops[0], Ops[1],
                                 8, 24,  9, 25, 10, 26, 11, 27,
                                12, 28, 13, 29, 14, 30, 15, 31);
    return true;
  case ALTIVEC_BUILTIN_VMRGLH:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 4, 12, 5, 13, 6, 14, 7, 15);
    return true;
  case ALTIVEC_BUILTIN_VMRGLW:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 2, 6, 3, 7);
    return true;
  case ALTIVEC_BUILTIN_ABS_V4SF: {
    /* and out sign bits */
    PackedType *v4i32 = PackedType::get(Type::Int32Ty, 4);
    Ops[0] = new BitCastInst(Ops[0], v4i32, Ops[0]->getName(),CurBB);
    Constant *C = ConstantInt::get(Type::Int32Ty, 0x7FFFFFFF);
    C = ConstantPacked::get(std::vector<Constant*>(4, C));
    Result = BinaryOperator::createAnd(Ops[0], C, "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case ALTIVEC_BUILTIN_ABS_V4SI:
  case ALTIVEC_BUILTIN_ABS_V8HI:
  case ALTIVEC_BUILTIN_ABS_V16QI: { /* iabs(x) -> smax(x, 0-x) */
    Result = BinaryOperator::createNeg(Ops[0], "tmp", CurBB);
    /* get the right smax intrinsic. */
    static Constant *smax[3];
    const PackedType *PTy = cast<PackedType>(ResultType);
    unsigned N = GetAltivecTypeNumFromType(PTy->getElementType());
    if (smax[N] == 0) {
      Module *M = CurBB->getParent()->getParent();
      smax[N] = M->getOrInsertFunction(std::string("llvm.ppc.altivec.vmaxs")+
                         GetAltivecLetterFromType(PTy->getElementType()),
                                      ResultType, ResultType, ResultType, NULL);
    }
    Result = new CallInst(smax[N], Ops[0], Result, "tmp", CurBB);
    return true;
  }
  case ALTIVEC_BUILTIN_ABSS_V4SI:
  case ALTIVEC_BUILTIN_ABSS_V8HI:
  case ALTIVEC_BUILTIN_ABSS_V16QI: { /* iabss(x) -> smax(x, satsub(0,x)) */
    static Constant *sxs[3], *smax[3];
    /* get the right satsub intrinsic. */
    const PackedType *PTy = cast<PackedType>(ResultType);
    unsigned N = GetAltivecTypeNumFromType(PTy->getElementType());
    if (sxs[N] == 0) {
      Module *M = CurBB->getParent()->getParent();
      sxs[N] = M->getOrInsertFunction(std::string("llvm.ppc.altivec.vsubs")+
                     GetAltivecLetterFromType(PTy->getElementType())+"s",
                                      ResultType, ResultType, ResultType, NULL);
    }
    Result = Constant::getNullValue(ResultType);
    Result = new CallInst(sxs[N], Result, Ops[0], "tmp", CurBB);
    /* get the right smax intrinsic. */
    if (smax[N] == 0) {
      Module *M = CurBB->getParent()->getParent();
      smax[N] = M->getOrInsertFunction(std::string("llvm.ppc.altivec.vmaxs")+
                         GetAltivecLetterFromType(PTy->getElementType()),
                                      ResultType, ResultType, ResultType, NULL);
    }
    Result = new CallInst(smax[N], Ops[0], Result, "tmp", CurBB);
    return true;
  }
  }

  return false;
}

/* APPLE LOCAL end LLVM (ENTIRE FILE!)  */

