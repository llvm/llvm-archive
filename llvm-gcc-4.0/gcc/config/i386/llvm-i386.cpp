/* APPLE LOCAL begin LLVM (ENTIRE FILE!)  */
/* High-level LLVM backend interface 
Copyright (C) 2005 Free Software Foundation, Inc.
Contributed by Evan Cheng (evan.cheng@apple.com)

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
// This is a C++ source file that implements specific llvm IA-32 ABI.
//===----------------------------------------------------------------------===//

#include "llvm-abi.h"
#include "llvm-internal.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/Module.h"

extern "C" {
#include "toplev.h"
}

/* TargetIntrinsicCastResult - This function just provides a frequently used
 * sequence for use inside TargetIntrinsicLower.
 */
static void TargetIntrinsicCastResult(Value *&Result, const Type *ResultType,
                                      bool ResIsSigned, bool ExpIsSigned,
                                      BasicBlock *CurBB) {
  Instruction::CastOps opcode =
    CastInst::getCastOpcode(Result, ResIsSigned, ResultType, ExpIsSigned);
  Result = CastInst::create(opcode, Result, ResultType, "tmp", CurBB);
}

/* IntrinsicOpIsSigned - This function determines if a given operand to the
 * intrinsic is signed or not.
 */
static bool IntrinsicOpIsSigned(SmallVector<tree, 8> &Args, unsigned OpNum) {
  return !TYPE_UNSIGNED(TREE_TYPE(Args[OpNum]));
}

/* TargetIntrinsicLower - For builtins that we want to expand to normal LLVM
 * code, emit the code now.  If we can handle the code, this macro should emit
 * the code, return true.
 */
bool TreeToLLVM::TargetIntrinsicLower(tree exp,
                                      unsigned FnCode,
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
  case IX86_BUILTIN_ADDPS:
  case IX86_BUILTIN_ADDPD:
  case IX86_BUILTIN_PADDB:
  case IX86_BUILTIN_PADDW:
  case IX86_BUILTIN_PADDD:
  case IX86_BUILTIN_PADDQ:
  case IX86_BUILTIN_PADDB128:
  case IX86_BUILTIN_PADDW128:
  case IX86_BUILTIN_PADDD128:
  case IX86_BUILTIN_PADDQ128:
    Result = BinaryOperator::createAdd(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_SUBPS:
  case IX86_BUILTIN_SUBPD:
  case IX86_BUILTIN_PSUBB:
  case IX86_BUILTIN_PSUBW:
  case IX86_BUILTIN_PSUBD:
  case IX86_BUILTIN_PSUBB128:
  case IX86_BUILTIN_PSUBW128:
  case IX86_BUILTIN_PSUBD128:
  case IX86_BUILTIN_PSUBQ128:
    Result = BinaryOperator::createSub(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_MULPS:
  case IX86_BUILTIN_MULPD:
  case IX86_BUILTIN_PMULLW:
  case IX86_BUILTIN_PMULLW128:
    Result = BinaryOperator::createMul(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_PSLLWI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    VectorType *v4i16 = VectorType::get(Type::Int16Ty, 4);
    static Constant *psllw = 0;
    if (psllw == 0) {
      Module *M = CurBB->getParent()->getParent();
      psllw = M->getOrInsertFunction("llvm.x86.mmx.psll.w",
                                     v4i16, v4i16, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psllw, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSLLWI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    VectorType *v8i16 = VectorType::get(Type::Int16Ty, 8);
    static Constant *psllw = 0;
    if (psllw == 0) {
      Module *M = CurBB->getParent()->getParent();
      psllw = M->getOrInsertFunction("llvm.x86.sse2.psll.w",
                                     v8i16, v8i16, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psllw, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSLLDI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    static Constant *pslld = 0;
    if (pslld == 0) {
      Module *M = CurBB->getParent()->getParent();
      pslld = M->getOrInsertFunction("llvm.x86.mmx.psll.d",
                                     v2i32, v2i32, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(pslld, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSLLDI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    static Constant *pslld = 0;
    if (pslld == 0) {
      Module *M = CurBB->getParent()->getParent();
      pslld = M->getOrInsertFunction("llvm.x86.sse2.psll.d",
                                     v4i32, v4i32, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(pslld, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSLLQI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    static Constant *psllq = 0;
    if (psllq == 0) {
      Module *M = CurBB->getParent()->getParent();
      psllq = M->getOrInsertFunction("llvm.x86.mmx.psll.q",
                                     v2i32, v2i32, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psllq, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSLLQI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    VectorType *v2i64 = VectorType::get(Type::Int64Ty, 2);
    static Constant *psllq = 0;
    if (psllq == 0) {
      Module *M = CurBB->getParent()->getParent();
      psllq = M->getOrInsertFunction("llvm.x86.sse2.psll.q",
                                     v2i64, v2i64, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psllq, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRLWI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    VectorType *v4i16 = VectorType::get(Type::Int16Ty, 4);
    static Constant *psrlw = 0;
    if (psrlw == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrlw = M->getOrInsertFunction("llvm.x86.mmx.psrl.w",
                                     v4i16, v4i16, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psrlw, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRLWI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    VectorType *v8i16 = VectorType::get(Type::Int16Ty, 8);
    static Constant *psrlw = 0;
    if (psrlw == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrlw = M->getOrInsertFunction("llvm.x86.sse2.psrl.w",
                                     v8i16, v8i16, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psrlw, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRLDI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    static Constant *psrld = 0;
    if (psrld == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrld = M->getOrInsertFunction("llvm.x86.mmx.psrl.d",
                                     v2i32, v2i32, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psrld, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRLDI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    static Constant *psrld = 0;
    if (psrld == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrld = M->getOrInsertFunction("llvm.x86.sse2.psrl.d",
                                     v4i32, v4i32, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psrld, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRLQI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    static Constant *psrlq = 0;
    if (psrlq == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrlq = M->getOrInsertFunction("llvm.x86.mmx.psrl.q",
                                     v2i32, v2i32, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psrlq, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRLQI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    VectorType *v2i64 = VectorType::get(Type::Int64Ty, 2);
    static Constant *psrlq = 0;
    if (psrlq == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrlq = M->getOrInsertFunction("llvm.x86.sse2.psrl.q",
                                     v2i64, v2i64, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psrlq, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRAWI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    VectorType *v4i16 = VectorType::get(Type::Int16Ty, 4);
    static Constant *psraw = 0;
    if (psraw == 0) {
      Module *M = CurBB->getParent()->getParent();
      psraw = M->getOrInsertFunction("llvm.x86.mmx.psra.w",
                                     v4i16, v4i16, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psraw, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRAWI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    VectorType *v8i16 = VectorType::get(Type::Int16Ty, 8);
    static Constant *psraw = 0;
    if (psraw == 0) {
      Module *M = CurBB->getParent()->getParent();
      psraw = M->getOrInsertFunction("llvm.x86.sse2.psra.w",
                                     v8i16, v8i16, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psraw, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRADI: {
    VectorType *v2i32 = VectorType::get(Type::Int32Ty, 2);
    static Constant *psrad = 0;
    if (psrad == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrad = M->getOrInsertFunction("llvm.x86.mmx.psra.d",
                                     v2i32, v2i32, v2i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, NULL);
    Result = new CallInst(psrad, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_PSRADI128: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    static Constant *psrad = 0;
    if (psrad == 0) {
      Module *M = CurBB->getParent()->getParent();
      psrad = M->getOrInsertFunction("llvm.x86.sse2.psra.d",
                                     v4i32, v4i32, v4i32, NULL);
    }
    Value *Undef = UndefValue::get(Type::Int32Ty);
    Ops[1] = BuildVector(Ops[1], Undef, Undef, Undef, NULL);
    Result = new CallInst(psrad, Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_DIVPS:
  case IX86_BUILTIN_DIVPD:
    Result = BinaryOperator::createFDiv(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_PAND:
  case IX86_BUILTIN_PAND128:
    Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_PANDN:
  case IX86_BUILTIN_PANDN128:
    Ops[0] = BinaryOperator::createNot(Ops[0], "tmp", CurBB);
    Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_POR:
  case IX86_BUILTIN_POR128:
    Result = BinaryOperator::createOr(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_PXOR:
  case IX86_BUILTIN_PXOR128:
    Result = BinaryOperator::createXor(Ops[0], Ops[1], "tmp", CurBB);
    return true;
  case IX86_BUILTIN_ANDPS:
  case IX86_BUILTIN_ORPS:
  case IX86_BUILTIN_XORPS:
  case IX86_BUILTIN_ANDNPS: {
    VectorType *v4i32 = VectorType::get(Type::Int32Ty, 4);
    Ops[0] = new BitCastInst(Ops[0], v4i32, "tmp", CurBB);
    Ops[1] = new BitCastInst(Ops[1], v4i32, "tmp", CurBB);
    switch (FnCode) {
      case IX86_BUILTIN_ANDPS:
        Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
        break;
      case IX86_BUILTIN_ORPS:
        Result = BinaryOperator::createOr (Ops[0], Ops[1], "tmp", CurBB);
         break;
      case IX86_BUILTIN_XORPS:
        Result = BinaryOperator::createXor(Ops[0], Ops[1], "tmp", CurBB);
        break;
      case IX86_BUILTIN_ANDNPS:
        Ops[0] = BinaryOperator::createNot(Ops[0], "tmp", CurBB);
        Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
        break;
    }
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_ANDPD:
  case IX86_BUILTIN_ORPD:
  case IX86_BUILTIN_XORPD:
  case IX86_BUILTIN_ANDNPD: {
    VectorType *v2i64 = VectorType::get(Type::Int64Ty, 2);
    Ops[0] = new BitCastInst(Ops[0], v2i64, "tmp", CurBB);
    Ops[1] = new BitCastInst(Ops[1], v2i64, "tmp", CurBB);
    switch (FnCode) {
      case IX86_BUILTIN_ANDPD:
        Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
        break;
      case IX86_BUILTIN_ORPD:
        Result = BinaryOperator::createOr (Ops[0], Ops[1], "tmp", CurBB);
         break;
      case IX86_BUILTIN_XORPD:
        Result = BinaryOperator::createXor(Ops[0], Ops[1], "tmp", CurBB);
        break;
      case IX86_BUILTIN_ANDNPD:
        Ops[0] = BinaryOperator::createNot(Ops[0], "tmp", CurBB);
        Result = BinaryOperator::createAnd(Ops[0], Ops[1], "tmp", CurBB);
        break;
    }
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_SHUFPS:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[2])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[1],
                                  ((EV & 0x03) >> 0),   ((EV & 0x0c) >> 2),
                                  ((EV & 0x30) >> 4)+4, ((EV & 0xc0) >> 6)+4);
    } else {
      error("%Hmask must be an immediate", &EXPR_LOCATION(exp));
      Result = Ops[0];
    }
    
    return true;
  case IX86_BUILTIN_PSHUFW:
  case IX86_BUILTIN_PSHUFD:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[1])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[0],
                                  ((EV & 0x03) >> 0),   ((EV & 0x0c) >> 2),
                                  ((EV & 0x30) >> 4),   ((EV & 0xc0) >> 6));
    } else {
      error("%Hmask must be an immediate", &EXPR_LOCATION(exp));
      Result = Ops[0];
    }
    return true;
  case IX86_BUILTIN_PSHUFHW:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[1])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[0],
                                  0, 1, 2, 3,
                                  ((EV & 0x03) >> 0)+4, ((EV & 0x0c) >> 2)+4,
                                  ((EV & 0x30) >> 4)+4, ((EV & 0xc0) >> 6)+4);
      return true;
    }
    return false;
  case IX86_BUILTIN_PSHUFLW:
    if (ConstantInt *Elt = dyn_cast<ConstantInt>(Ops[1])) {
      int EV = Elt->getZExtValue();
      Result = BuildVectorShuffle(Ops[0], Ops[0],
                                  ((EV & 0x03) >> 0),   ((EV & 0x0c) >> 2),
                                  ((EV & 0x30) >> 4),   ((EV & 0xc0) >> 6),
                                  4, 5, 6, 7);
    } else {
      error("%Hmask must be an immediate", &EXPR_LOCATION(exp));
      Result = Ops[0];
    }
    
    return true;
  case IX86_BUILTIN_PUNPCKHBW:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 4, 12, 5, 13,
                                                6, 14, 7, 15);
    return true;
  case IX86_BUILTIN_PUNPCKHWD:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 2, 6, 3, 7);
    return true;
  case IX86_BUILTIN_PUNPCKHDQ:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 1, 3);
    return true;
  case IX86_BUILTIN_PUNPCKLBW:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0,  8, 1,  9,
                                                2, 10, 3, 11);
    return true;
  case IX86_BUILTIN_PUNPCKLWD:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 4, 1, 5);
    return true;
  case IX86_BUILTIN_PUNPCKLDQ:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 2);
    return true;
  case IX86_BUILTIN_PUNPCKHBW128:
    Result = BuildVectorShuffle(Ops[0], Ops[1],  8, 24,  9, 25,
                                                10, 26, 11, 27,
                                                12, 28, 13, 29,
                                                14, 30, 15, 31);
    return true;
  case IX86_BUILTIN_PUNPCKHWD128:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 4, 12, 5, 13, 6, 14, 7, 15);
    return true;
  case IX86_BUILTIN_PUNPCKHDQ128:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 2, 6, 3, 7);
    return true;
  case IX86_BUILTIN_PUNPCKLBW128:
    Result = BuildVectorShuffle(Ops[0], Ops[1],  0, 16,  1, 17,
                                                 2, 18,  3, 19,
                                                 4, 20,  5, 21,
                                                 6, 22,  7, 23);
    return true;
  case IX86_BUILTIN_PUNPCKLWD128:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 8, 1, 9, 2, 10, 3, 11);
    return true;
  case IX86_BUILTIN_PUNPCKLDQ128:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 4, 1, 5);
    return true;
  case IX86_BUILTIN_UNPCKHPS:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 2, 6, 3, 7);
    return true;
  case IX86_BUILTIN_UNPCKLPS:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 4, 1, 5);
    return true;
  case IX86_BUILTIN_MOVHLPS:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 6, 7, 2, 3);
    return true;
  case IX86_BUILTIN_MOVLHPS:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 1, 4, 5);
    return true;
  case IX86_BUILTIN_MOVSS:
    Result = BuildVectorShuffle(Ops[0], Ops[1], 4, 1, 2, 3);
    return true;
  case IX86_BUILTIN_LOADQ: {
    PointerType *f64Ptr = PointerType::get(Type::DoubleTy);
    Value *Zero = ConstantFP::get(Type::DoubleTy, 0.0);
    Ops[0] = new BitCastInst(Ops[0], f64Ptr, "tmp", CurBB);
    Ops[0] = new LoadInst(Ops[0], "tmp", false, CurBB);
    Result = BuildVector(Ops[0], Zero, NULL);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_LOADHPS: {
    PointerType *f64Ptr = PointerType::get(Type::DoubleTy);
    Ops[1] = new BitCastInst(Ops[1], f64Ptr, "tmp", CurBB);
    Value *Load = new LoadInst(Ops[1], "tmp", false, CurBB);
    Ops[1] = BuildVector(Load, UndefValue::get(Type::DoubleTy), NULL);
    Instruction::CastOps opcode = CastInst::getCastOpcode(Ops[1],
      IntrinsicOpIsSigned(Args, 1), ResultType, ExpIsSigned);
    Ops[1] = CastInst::create(opcode, Ops[1], ResultType, "tmp", CurBB);
    Result = BuildVectorShuffle(Ops[0], Ops[1], 0, 1, 4, 5);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_LOADLPS: {
    PointerType *f64Ptr = PointerType::get(Type::DoubleTy);
    Ops[1] = new BitCastInst(Ops[1], f64Ptr, "tmp", CurBB);
    Value *Load = new LoadInst(Ops[1], "tmp", false, CurBB);
    Ops[1] = BuildVector(Load, UndefValue::get(Type::DoubleTy), NULL);
    Instruction::CastOps opcode = CastInst::getCastOpcode(Ops[1],
      IntrinsicOpIsSigned(Args, 1), ResultType, ExpIsSigned);
    Ops[1] = CastInst::create(opcode, Ops[1], ResultType, "tmp", CurBB);
    Result = BuildVectorShuffle(Ops[0], Ops[1], 4, 5, 2, 3);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_STOREHPS: {
    VectorType *v2f64 = VectorType::get(Type::DoubleTy, 2);
    PointerType *f64Ptr = PointerType::get(Type::DoubleTy);
    Ops[0] = new BitCastInst(Ops[0], f64Ptr, "tmp", CurBB);
    Value *Idx = ConstantInt::get(Type::Int32Ty, 1);
    Ops[1] = new BitCastInst(Ops[1], v2f64, "tmp", CurBB);
    Ops[1] = new ExtractElementInst(Ops[1], Idx, "tmp", CurBB);
    Result = new StoreInst(Ops[1], Ops[0], false, CurBB);
    return true;
  }
  case IX86_BUILTIN_STORELPS: {
    VectorType *v2f64 = VectorType::get(Type::DoubleTy, 2);
    PointerType *f64Ptr = PointerType::get(Type::DoubleTy);
    Ops[0] = new BitCastInst(Ops[0], f64Ptr, "tmp", CurBB);
    Value *Idx = ConstantInt::get(Type::Int32Ty, 0);
    Ops[1] = new BitCastInst(Ops[1], v2f64, "tmp", CurBB);
    Ops[1] = new ExtractElementInst(Ops[1], Idx, "tmp", CurBB);
    Result = new StoreInst(Ops[1], Ops[0], false, CurBB);
    return true;
  }
  case IX86_BUILTIN_MOVSHDUP:
    Result = BuildVectorShuffle(Ops[0], Ops[0], 1, 1, 3, 3);
    return true;
  case IX86_BUILTIN_MOVSLDUP:
    Result = BuildVectorShuffle(Ops[0], Ops[0], 0, 0, 2, 2);
    return true;
  case IX86_BUILTIN_VEC_INIT_V2SI:
    for (unsigned i = 0; i < 2; ++i)
      Ops[i] = CastInst::createIntegerCast(Ops[i], Type::Int32Ty, false, "tmp",
                                           CurBB);

    Result = BuildVector(Ops[0], Ops[1], NULL);
    return true;
  case IX86_BUILTIN_VEC_INIT_V4HI:
    for (unsigned i = 0; i < 4; ++i)
      Ops[i] = CastInst::createIntegerCast(Ops[i], Type::Int16Ty, false, "tmp",
                                           CurBB);

    Result = BuildVector(Ops[0], Ops[1], Ops[2], Ops[3], NULL);
    return true;
  case IX86_BUILTIN_VEC_INIT_V8QI: {
    for (unsigned i = 0; i < 8; ++i)
      Ops[i] = CastInst::createIntegerCast(Ops[i], Type::Int8Ty, false, "tmp",
                                           CurBB);

    Result = BuildVector(Ops[0], Ops[1], Ops[2], Ops[3],
                         Ops[4], Ops[5], Ops[6], Ops[7], NULL);
    return true;
  }
  case IX86_BUILTIN_VEC_EXT_V2SI:
  case IX86_BUILTIN_VEC_EXT_V4HI:
  case IX86_BUILTIN_VEC_EXT_V2DF:
  case IX86_BUILTIN_VEC_EXT_V4SI:
  case IX86_BUILTIN_VEC_EXT_V4SF:
  case IX86_BUILTIN_VEC_EXT_V8HI: {
    Ops[1] = CastInst::createIntegerCast(Ops[1], Type::Int32Ty, false, "tmp",
                                         CurBB);
    Result = new ExtractElementInst(Ops[0], Ops[1], "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_VEC_SET_V8HI: {
    Instruction::CastOps opcode = CastInst::getCastOpcode(Ops[1],
      IntrinsicOpIsSigned(Args, 1),  Type::Int16Ty, true);
    Ops[1] = CastInst::create(opcode, Ops[1], Type::Int16Ty, "tmp", CurBB);
    opcode = CastInst::getCastOpcode(Ops[2],
      IntrinsicOpIsSigned(Args, 2), Type::Int32Ty, false);
    Ops[2] = CastInst::create(opcode, Ops[2], Type::Int32Ty,  "tmp", CurBB);
    Result = new InsertElementInst(Ops[0], Ops[1], Ops[2], "tmp", CurBB);
    return true;
  }
  case IX86_BUILTIN_CMPEQPS:
  case IX86_BUILTIN_CMPLTPS:
  case IX86_BUILTIN_CMPLEPS:
  case IX86_BUILTIN_CMPGTPS:
  case IX86_BUILTIN_CMPGEPS:
  case IX86_BUILTIN_CMPNEQPS:
  case IX86_BUILTIN_CMPNLTPS:
  case IX86_BUILTIN_CMPNLEPS:
  case IX86_BUILTIN_CMPNGTPS:
  case IX86_BUILTIN_CMPNGEPS:
  case IX86_BUILTIN_CMPORDPS:
  case IX86_BUILTIN_CMPUNORDPS: {
    VectorType *v4f32 = VectorType::get(Type::FloatTy, 4);
    static Constant *cmpps = 0;
    if (cmpps == 0) {
      Module *M = CurBB->getParent()->getParent();
      cmpps = M->getOrInsertFunction("llvm.x86.sse.cmp.ps",
                                     v4f32, v4f32, v4f32, Type::Int8Ty, NULL);
    }
    bool flip = false;
    Value *Pred = 0;
    switch (FnCode) {
      case IX86_BUILTIN_CMPEQPS:
        Pred = ConstantInt::get(Type::Int8Ty, 0);
        break;
      case IX86_BUILTIN_CMPLTPS:
        Pred = ConstantInt::get(Type::Int8Ty, 1);
        break;
      case IX86_BUILTIN_CMPGTPS:
        Pred = ConstantInt::get(Type::Int8Ty, 1);
        flip = true;
        break;
      case IX86_BUILTIN_CMPLEPS:
        Pred = ConstantInt::get(Type::Int8Ty, 2);
        break;
      case IX86_BUILTIN_CMPGEPS:
        Pred = ConstantInt::get(Type::Int8Ty, 2);
        flip = true;
        break;
      case IX86_BUILTIN_CMPUNORDPS:
        Pred = ConstantInt::get(Type::Int8Ty, 3);
        break;
      case IX86_BUILTIN_CMPNEQPS:
        Pred = ConstantInt::get(Type::Int8Ty, 4);
        break;
      case IX86_BUILTIN_CMPNLTPS:
        Pred = ConstantInt::get(Type::Int8Ty, 5);
        break;
      case IX86_BUILTIN_CMPNGTPS:
        Pred = ConstantInt::get(Type::Int8Ty, 5);
        flip = true;
        break;
      case IX86_BUILTIN_CMPNLEPS:
        Pred = ConstantInt::get(Type::Int8Ty, 6);
        break;
      case IX86_BUILTIN_CMPNGEPS:
        Pred = ConstantInt::get(Type::Int8Ty, 6);
        flip = true;
        break;
      case IX86_BUILTIN_CMPORDPS:
        Pred = ConstantInt::get(Type::Int8Ty, 7);
        break;
    }
    Value *Arg0 = new BitCastInst(Ops[0], v4f32, "tmp", CurBB);
    Value *Arg1 = new BitCastInst(Ops[1], v4f32, "tmp", CurBB);
    if (flip) std::swap(Arg0, Arg1);
    Value *CallOps[3] = { Arg0, Arg1, Pred };
    Result = new CallInst(cmpps, CallOps, 3, "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_CMPEQSS:
  case IX86_BUILTIN_CMPLTSS:
  case IX86_BUILTIN_CMPLESS:
  case IX86_BUILTIN_CMPNEQSS:
  case IX86_BUILTIN_CMPNLTSS:
  case IX86_BUILTIN_CMPNLESS:
  case IX86_BUILTIN_CMPNGTSS:
  case IX86_BUILTIN_CMPNGESS:
  case IX86_BUILTIN_CMPORDSS:
  case IX86_BUILTIN_CMPUNORDSS: {
    VectorType *v4f32 = VectorType::get(Type::FloatTy, 4);
    static Constant *cmpss = 0;
    if (cmpss == 0) {
      Module *M = CurBB->getParent()->getParent();
      cmpss = M->getOrInsertFunction("llvm.x86.sse.cmp.ss",
                                     v4f32, v4f32, v4f32, Type::Int8Ty, NULL);
    }
    Value *Pred = 0;
    switch (FnCode) {
      case IX86_BUILTIN_CMPEQSS:
        Pred = ConstantInt::get(Type::Int8Ty, 0);
        break;
      case IX86_BUILTIN_CMPLTSS:
        Pred = ConstantInt::get(Type::Int8Ty, 1);
        break;
      case IX86_BUILTIN_CMPLESS:
        Pred = ConstantInt::get(Type::Int8Ty, 2);
        break;
      case IX86_BUILTIN_CMPUNORDSS:
        Pred = ConstantInt::get(Type::Int8Ty, 3);
        break;
      case IX86_BUILTIN_CMPNEQSS:
        Pred = ConstantInt::get(Type::Int8Ty, 4);
        break;
      case IX86_BUILTIN_CMPNLTSS:
        Pred = ConstantInt::get(Type::Int8Ty, 5);
        break;
      case IX86_BUILTIN_CMPNLESS:
        Pred = ConstantInt::get(Type::Int8Ty, 6);
        break;
      case IX86_BUILTIN_CMPORDSS:
        Pred = ConstantInt::get(Type::Int8Ty, 7);
        break;
    }
    Value *Arg0 = new BitCastInst(Ops[0], v4f32, "tmp", CurBB);
    Value *Arg1 = new BitCastInst(Ops[1], v4f32, "tmp", CurBB);
    
    Value *CallOps[3] = { Arg0, Arg1, Pred };
    Result = new CallInst(cmpss, CallOps, 3, "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_CMPEQPD:
  case IX86_BUILTIN_CMPLTPD:
  case IX86_BUILTIN_CMPLEPD:
  case IX86_BUILTIN_CMPGTPD:
  case IX86_BUILTIN_CMPGEPD:
  case IX86_BUILTIN_CMPNEQPD:
  case IX86_BUILTIN_CMPNLTPD:
  case IX86_BUILTIN_CMPNLEPD:
  case IX86_BUILTIN_CMPNGTPD:
  case IX86_BUILTIN_CMPNGEPD:
  case IX86_BUILTIN_CMPORDPD:
  case IX86_BUILTIN_CMPUNORDPD: {
    VectorType *v2f64 = VectorType::get(Type::DoubleTy, 2);
    static Constant *cmpps = 0;
    if (cmpps == 0) {
      Module *M = CurBB->getParent()->getParent();
      cmpps = M->getOrInsertFunction("llvm.x86.sse2.cmp.pd",
                                     v2f64, v2f64, v2f64, Type::Int8Ty, NULL);
    }
    bool flip = false;
    Value *Pred = 0;
    switch (FnCode) {
      case IX86_BUILTIN_CMPEQPD:
        Pred = ConstantInt::get(Type::Int8Ty, 0);
        break;
      case IX86_BUILTIN_CMPLTPD:
        Pred = ConstantInt::get(Type::Int8Ty, 1);
        break;
      case IX86_BUILTIN_CMPGTPD:
        Pred = ConstantInt::get(Type::Int8Ty, 1);
        flip = true;
        break;
      case IX86_BUILTIN_CMPLEPD:
        Pred = ConstantInt::get(Type::Int8Ty, 2);
        break;
      case IX86_BUILTIN_CMPGEPD:
        Pred = ConstantInt::get(Type::Int8Ty, 2);
        flip = true;
        break;
      case IX86_BUILTIN_CMPUNORDPD:
        Pred = ConstantInt::get(Type::Int8Ty, 3);
        break;
      case IX86_BUILTIN_CMPNEQPD:
        Pred = ConstantInt::get(Type::Int8Ty, 4);
        break;
      case IX86_BUILTIN_CMPNLTPD:
        Pred = ConstantInt::get(Type::Int8Ty, 5);
        break;
      case IX86_BUILTIN_CMPNGTPD:
        Pred = ConstantInt::get(Type::Int8Ty, 5);
        flip = true;
        break;
      case IX86_BUILTIN_CMPNLEPD:
        Pred = ConstantInt::get(Type::Int8Ty, 6);
        break;
      case IX86_BUILTIN_CMPNGEPD:
        Pred = ConstantInt::get(Type::Int8Ty, 6);
        flip = true;
        break;
      case IX86_BUILTIN_CMPORDPD:
        Pred = ConstantInt::get(Type::Int8Ty, 7);
        break;
    }
    Value *Arg0 = new BitCastInst(Ops[0], v2f64, "tmp", CurBB);
    Value *Arg1 = new BitCastInst(Ops[1], v2f64, "tmp", CurBB);
    if (flip) std::swap(Arg0, Arg1);

    Value *CallOps[3] = { Arg0, Arg1, Pred };
    Result = new CallInst(cmpps, CallOps, 3, "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_CMPEQSD:
  case IX86_BUILTIN_CMPLTSD:
  case IX86_BUILTIN_CMPLESD:
  case IX86_BUILTIN_CMPNEQSD:
  case IX86_BUILTIN_CMPNLTSD:
  case IX86_BUILTIN_CMPNLESD:
  case IX86_BUILTIN_CMPORDSD:
  case IX86_BUILTIN_CMPUNORDSD: {
    VectorType *v2f64 = VectorType::get(Type::DoubleTy, 2);
    static Constant *cmpss = 0;
    if (cmpss == 0) {
      Module *M = CurBB->getParent()->getParent();
      cmpss = M->getOrInsertFunction("llvm.x86.sse2.cmp.sd",
                                     v2f64, v2f64, v2f64, Type::Int8Ty, NULL);
    }
    Value *Pred = 0;
    switch (FnCode) {
      case IX86_BUILTIN_CMPEQSD:
        Pred = ConstantInt::get(Type::Int8Ty, 0);
        break;
      case IX86_BUILTIN_CMPLTSD:
        Pred = ConstantInt::get(Type::Int8Ty, 1);
        break;
      case IX86_BUILTIN_CMPLESD:
        Pred = ConstantInt::get(Type::Int8Ty, 2);
        break;
      case IX86_BUILTIN_CMPUNORDSD:
        Pred = ConstantInt::get(Type::Int8Ty, 3);
        break;
      case IX86_BUILTIN_CMPNEQSD:
        Pred = ConstantInt::get(Type::Int8Ty, 4);
        break;
      case IX86_BUILTIN_CMPNLTSD:
        Pred = ConstantInt::get(Type::Int8Ty, 5);
        break;
      case IX86_BUILTIN_CMPNLESD:
        Pred = ConstantInt::get(Type::Int8Ty, 6);
        break;
      case IX86_BUILTIN_CMPORDSD:
        Pred = ConstantInt::get(Type::Int8Ty, 7);
        break;
    }

    Value *Arg0 = new BitCastInst(Ops[0], v2f64, "tmp", CurBB);
    Value *Arg1 = new BitCastInst(Ops[1], v2f64, "tmp", CurBB);
    Value *CallOps[3] = { Arg0, Arg1, Pred };
    Result = new CallInst(cmpss, CallOps, 3, "tmp", CurBB);
    TargetIntrinsicCastResult(Result, ResultType,
                              ResIsSigned, ExpIsSigned, CurBB);
    return true;
  }
  case IX86_BUILTIN_LDMXCSR: {
    PointerType *u32Ptr = PointerType::get(Type::Int32Ty);
    static Constant *ldmxcsr = 0;
    if (ldmxcsr == 0) {
      Module *M = CurBB->getParent()->getParent();
      ldmxcsr = M->getOrInsertFunction("llvm.x86.sse.ldmxcsr",
                                       Type::VoidTy, u32Ptr, NULL);
    }
    Value *Ptr = CreateTemporary(Type::Int32Ty);
    new StoreInst(Ops[0], Ptr, false, CurBB);
    Result = new CallInst(ldmxcsr, Ptr, "", CurBB);
    return true;
  }
  case IX86_BUILTIN_STMXCSR: {
    PointerType *u32Ptr = PointerType::get(Type::Int32Ty);
    static Constant *stmxcsr = 0;
    if (stmxcsr == 0) {
      Module *M = CurBB->getParent()->getParent();
      stmxcsr = M->getOrInsertFunction("llvm.x86.sse.stmxcsr",
                                       Type::VoidTy, u32Ptr, NULL);
    }
    Value *Ptr = CreateTemporary(Type::Int32Ty);
    new CallInst(stmxcsr, Ptr, "", CurBB);
    Result = new LoadInst(Ptr, "tmp", false, CurBB);
    return true;
  }
  }

  return false;
}
