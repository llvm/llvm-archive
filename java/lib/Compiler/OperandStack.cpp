//===-- OperandStack.cpp - Java operand stack -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the abstraction of a Java operand stack. We
// model the java operand stack as a stack of LLVM allocas.
//
//===----------------------------------------------------------------------===//

#include "OperandStack.h"
#include <llvm/BasicBlock.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/StringExtras.h>

using namespace llvm::Java;

void OperandStack::push(Value* value, BasicBlock* insertAtEnd)
{
  assert(currentDepth < TheStack.size() && "Pushing to a full stack!");

  const Type* valueTy = value->getType();
  // Values of jboolean, jbyte, jchar and jshort are extended to a
  // jint when pushed on the operand stack.
  if (valueTy == Type::BoolTy ||
      valueTy == Type::SByteTy ||
      valueTy == Type::UShortTy ||
      valueTy == Type::ShortTy)
    value = new CastInst(value, Type::IntTy, "int-extend", insertAtEnd);

  // If we don't have an alloca already for this slot create one
  if (!TheStack[currentDepth] ||
      TheStack[currentDepth]->getAllocatedType() != value->getType()) {
    // Insert the alloca at the beginning of the entry block.
    BasicBlock* entry = &insertAtEnd->getParent()->getEntryBlock();
    if (entry->empty())
      TheStack[currentDepth] =
        new AllocaInst(value->getType(),
                       NULL,
                       "opStack" + utostr(currentDepth),
                       entry);
    else
      TheStack[currentDepth] =
        new AllocaInst(value->getType(),
                       NULL,
                       "opStack" + utostr(currentDepth),
                       &entry->front());
  }

  new StoreInst(value, TheStack[currentDepth++], insertAtEnd);
}

llvm::Value* OperandStack::pop(BasicBlock* insertAtEnd)
{
  assert(currentDepth != 0 && "Popping from an empty stack!");

  return new LoadInst(TheStack[--currentDepth], "pop", insertAtEnd);
}
