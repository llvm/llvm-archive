//===-- Locals.h - Java locals ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the abstraction of Java locals. We model the
// locals as an array of lazily created allocas.
//
//===----------------------------------------------------------------------===//

#include "Locals.h"
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/StringExtras.h>

using namespace llvm::Java;

Locals::Locals(unsigned maxLocals)
  : TheLocals(maxLocals)
{

}

void Locals::store(unsigned i, Value* value, BasicBlock* insertAtEnd)
{
  if (!TheLocals[i] ||
      TheLocals[i]->getType()->getElementType() != value->getType())
    TheLocals[i] = new AllocaInst(value->getType(),
				  NULL,
				  "local" + utostr(i),
				  insertAtEnd);

  new StoreInst(value, TheLocals[i], insertAtEnd);
}

llvm::Value* Locals::load(unsigned i, BasicBlock* insertAtEnd)
{
  assert(TheLocals[i] && "Attempt to load a non initialized global!");
  return new LoadInst(TheLocals[i], "load", insertAtEnd);
}
