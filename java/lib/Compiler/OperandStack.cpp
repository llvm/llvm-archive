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
#include "Support.h"
#include <llvm/BasicBlock.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Java/Compiler.h>

using namespace llvm;
using namespace llvm::Java;

void OperandStack::copySlots(const SlotMap& src,
                             SlotMap& dst,
                             BasicBlock* insertAtEnd)
{
  SlotMap::const_iterator it = src.begin();
  SlotMap::const_iterator end = src.end();

  for (; it != end; ++it) {
    AllocaInst* srcSlot = it->second;
    AllocaInst* dstSlot = getOrCreateSlot(dst, it->first, insertAtEnd);
    Value* value = new LoadInst(srcSlot, "tmp", insertAtEnd);
    new StoreInst(value, dstSlot, insertAtEnd);
  }
}

llvm::AllocaInst* OperandStack::getOrCreateSlot(SlotMap& slotMap,
                                                const Type* type,
                                                BasicBlock* bb)
{
  SlotMap::iterator it = slotMap.find(type);

  if (it == slotMap.end()) {
    // Insert the alloca at the beginning of the entry block.
    BasicBlock* entry = &bb->getParent()->getEntryBlock();
    AllocaInst* alloca = new AllocaInst(type, NULL, "opStack", entry);
    it = slotMap.insert(it, std::make_pair(type, alloca));
  }

  return it->second;
}

void OperandStack::push(Value* value, BasicBlock* insertAtEnd)
{
  assert(currentDepth < TheStack.size() && "Pushing to a full stack!");
  const Type* valueTy = value->getType();
//   std::cerr << "PUSH(" << insertAtEnd << "/"
//             << insertAtEnd->getParent()->getName() << " " << TheStack.size()
//             << ") Depth: " << currentDepth << " type: " << *valueTy << '\n';
  const Type* storageTy = getStorageType(valueTy);
  if (valueTy != storageTy)
    value = new CastInst(value, storageTy, "to-storage-type", insertAtEnd);

  SlotMap& slotMap = TheStack[currentDepth];
  AllocaInst* slot = getOrCreateSlot(slotMap, storageTy, insertAtEnd);
  new StoreInst(value, slot, insertAtEnd);
  currentDepth += 1 + isTwoSlotType(storageTy);
  assert(currentDepth < TheStack.size() && "Pushed more than max stack depth!");
}

llvm::Value* OperandStack::pop(const Type* valueTy, BasicBlock* insertAtEnd)
{
  const Type* storageTy = getStorageType(valueTy);

  assert(currentDepth != 0 && "Popping from an empty stack!");
  currentDepth -= 1 + isTwoSlotType(storageTy);
//   std::cerr << "POP(" << insertAtEnd->getName() << "/"
//             << insertAtEnd->getParent()->getName() << " " << TheStack.size()
//             << ")  Depth: " << currentDepth << " type: " << *valueTy << '\n';

  SlotMap& slotMap = TheStack[currentDepth];
  SlotMap::iterator it = slotMap.find(storageTy);

  assert(it != slotMap.end() && "Type mismatch on operand stack!");
  Value* value = new LoadInst(it->second, "pop", insertAtEnd);
  if (valueTy != storageTy)
    value = new CastInst(value, valueTy, "from-storage-type", insertAtEnd);
  return value;
}

/// ..., value -> ...
void OperandStack::do_pop(BasicBlock* insertAtEnd)
{
  assert(currentDepth != 0 && "Popping from an empty stack!");
  --currentDepth;
}

/// ..., value2, value1 -> ...
/// ..., value -> ...
void OperandStack::do_pop2(BasicBlock* insertAtEnd)
{
  do_pop(insertAtEnd);
  do_pop(insertAtEnd);
}

/// ..., value -> ..., value, value
void OperandStack::do_dup(BasicBlock* insertAtEnd)
{
  assert(currentDepth != 0 && "Popping from an empty stack!");
  copySlots(TheStack[currentDepth-1], TheStack[currentDepth], insertAtEnd);
  ++currentDepth;
}

/// ..., value2, value1 -> ..., value1, value2, value1
void OperandStack::do_dup_x1(BasicBlock* insertAtEnd)
{
  copySlots(TheStack[currentDepth-1], TheStack[currentDepth], insertAtEnd);
  copySlots(TheStack[currentDepth-2], TheStack[currentDepth-1], insertAtEnd);
  copySlots(TheStack[currentDepth], TheStack[currentDepth-2], insertAtEnd);
  ++currentDepth;
}

/// ..., value3, value2, value1 -> ..., value1, value3, value2, value1
/// ..., value2, value1 -> ..., value1, value2, value1
void OperandStack::do_dup_x2(BasicBlock* insertAtEnd)
{
  copySlots(TheStack[currentDepth-1], TheStack[currentDepth], insertAtEnd);
  copySlots(TheStack[currentDepth-2], TheStack[currentDepth-1], insertAtEnd);
  copySlots(TheStack[currentDepth-3], TheStack[currentDepth-2], insertAtEnd);
  copySlots(TheStack[currentDepth], TheStack[currentDepth-3], insertAtEnd);
  ++currentDepth;
}

/// ..., value2, value1 -> ..., value2, value1, value2, value1
void OperandStack::do_dup2(BasicBlock* insertAtEnd)
{
  copySlots(TheStack[currentDepth-2], TheStack[currentDepth], insertAtEnd);
  copySlots(TheStack[currentDepth-1], TheStack[currentDepth+1], insertAtEnd);
  currentDepth += 2;
}

/// ..., value3, value2, value1 -> ..., value2, value1, value3, value2, value1
/// ..., value2, value1 -> ..., value1, value2, value1
void OperandStack::do_dup2_x1(BasicBlock* insertAtEnd)
{
  copySlots(TheStack[currentDepth-1], TheStack[currentDepth+1], insertAtEnd);
  copySlots(TheStack[currentDepth-2], TheStack[currentDepth], insertAtEnd);
  copySlots(TheStack[currentDepth-3], TheStack[currentDepth-1], insertAtEnd);
  copySlots(TheStack[currentDepth+1], TheStack[currentDepth-2], insertAtEnd);
  copySlots(TheStack[currentDepth], TheStack[currentDepth-3], insertAtEnd);
  currentDepth += 2;
}

/// ..., value4, value3, value2, value1 -> ..., value2, value1, value4, value3, value2, value1
/// ..., value3, value2, value1 -> ..., value1, value3, value2, value1
/// ..., value3, value2, value1 -> ..., value2, value1, value3, value2, value1
/// ..., value2, value1 -> ..., value1, value2, value1
void OperandStack::do_dup2_x2(BasicBlock* insertAtEnd)
{
  copySlots(TheStack[currentDepth-1], TheStack[currentDepth+1], insertAtEnd);
  copySlots(TheStack[currentDepth-2], TheStack[currentDepth], insertAtEnd);
  copySlots(TheStack[currentDepth-3], TheStack[currentDepth-1], insertAtEnd);
  copySlots(TheStack[currentDepth-4], TheStack[currentDepth-2], insertAtEnd);
  copySlots(TheStack[currentDepth+1], TheStack[currentDepth-3], insertAtEnd);
  copySlots(TheStack[currentDepth], TheStack[currentDepth-4], insertAtEnd);
  currentDepth += 2;
}

void OperandStack::do_swap(BasicBlock* insertAtEnd)
{
  SlotMap tmp;
  copySlots(TheStack[currentDepth-1], tmp, insertAtEnd);
  copySlots(TheStack[currentDepth-2], TheStack[currentDepth-1], insertAtEnd);
  copySlots(tmp, TheStack[currentDepth-2], insertAtEnd);
}
