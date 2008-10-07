/* LLVM LOCAL begin (ENTIRE FILE!)  */
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
// This is a C++ source file that implements specific llvm alpha ABI.
//===----------------------------------------------------------------------===//

#include "llvm-abi.h"
#include "llvm-internal.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/Module.h"

extern "C" {
#include "toplev.h"
}

enum alpha_builtin
{
  ALPHA_BUILTIN_CMPBGE,
  ALPHA_BUILTIN_EXTBL,
  ALPHA_BUILTIN_EXTWL,
  ALPHA_BUILTIN_EXTLL,
  ALPHA_BUILTIN_EXTQL,
  ALPHA_BUILTIN_EXTWH,
  ALPHA_BUILTIN_EXTLH,
  ALPHA_BUILTIN_EXTQH,
  ALPHA_BUILTIN_INSBL,
  ALPHA_BUILTIN_INSWL,
  ALPHA_BUILTIN_INSLL,
  ALPHA_BUILTIN_INSQL,
  ALPHA_BUILTIN_INSWH,
  ALPHA_BUILTIN_INSLH,
  ALPHA_BUILTIN_INSQH,
  ALPHA_BUILTIN_MSKBL,
  ALPHA_BUILTIN_MSKWL,
  ALPHA_BUILTIN_MSKLL,
  ALPHA_BUILTIN_MSKQL,
  ALPHA_BUILTIN_MSKWH,
  ALPHA_BUILTIN_MSKLH,
  ALPHA_BUILTIN_MSKQH,
  ALPHA_BUILTIN_UMULH,
  ALPHA_BUILTIN_ZAP,
  ALPHA_BUILTIN_ZAPNOT,
  ALPHA_BUILTIN_AMASK,
  ALPHA_BUILTIN_IMPLVER,
  ALPHA_BUILTIN_RPCC,
  ALPHA_BUILTIN_THREAD_POINTER,
  ALPHA_BUILTIN_SET_THREAD_POINTER,

  /* TARGET_MAX */
  ALPHA_BUILTIN_MINUB8,
  ALPHA_BUILTIN_MINSB8,
  ALPHA_BUILTIN_MINUW4,
  ALPHA_BUILTIN_MINSW4,
  ALPHA_BUILTIN_MAXUB8,
  ALPHA_BUILTIN_MAXSB8,
  ALPHA_BUILTIN_MAXUW4,
  ALPHA_BUILTIN_MAXSW4,
  ALPHA_BUILTIN_PERR,
  ALPHA_BUILTIN_PKLB,
  ALPHA_BUILTIN_PKWB,
  ALPHA_BUILTIN_UNPKBL,
  ALPHA_BUILTIN_UNPKBW,

  /* TARGET_CIX */
  ALPHA_BUILTIN_CTTZ,
  ALPHA_BUILTIN_CTLZ,
  ALPHA_BUILTIN_CTPOP,

  ALPHA_BUILTIN_max
};

/* TargetIntrinsicLower - For builtins that we want to expand to normal LLVM
 * code, emit the code now.  If we can handle the code, this macro should emit
 * the code, return true.
 */
bool TreeToLLVM::TargetIntrinsicLower(tree exp,
                                      unsigned FnCode,
                                      const MemRef *DestLoc,
                                      Value *&Result,
                                      const Type *ResultType,
                                      std::vector<Value*> &Ops) {
  switch (FnCode) {
  case ALPHA_BUILTIN_UMULH: {
    Function *f =
      Intrinsic::getDeclaration(TheModule, Intrinsic::alpha_umulh);
    Value *Arg0 = Ops[0];
    Value *Arg1 = Ops[1];
    Value *CallOps[2] = { Arg0, Arg1};
    Result = Builder.CreateCall(f, CallOps, CallOps+2, "tmp");
    Result = Builder.CreateBitCast(Result, ResultType, "tmp");
    return true;
  }
  default: break;
  }

  return false;
}

/* LLVM LOCAL end (ENTIRE FILE!)  */
