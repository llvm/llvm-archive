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
#include "Support.h"
#include <llvm/BasicBlock.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Java/Compiler.h>

using namespace llvm;
using namespace llvm::Java;

Locals::Locals(unsigned maxLocals)
  : TheLocals(maxLocals)
{

}

void Locals::store(unsigned i, Value* value, BasicBlock* insertAtEnd)
{
  const Type* valueTy = value->getType();
  const Type* storageTy = getStorageType(valueTy);
  if (valueTy != storageTy)
    value = new CastInst(value, storageTy, "to-storage-type", insertAtEnd);

  SlotMap& slotMap = TheLocals[i];
  SlotMap::iterator it = slotMap.find(storageTy);

  if (it == slotMap.end()) {
    // Insert the alloca at the beginning of the entry block.
    BasicBlock& entry = insertAtEnd->getParent()->getEntryBlock();
    assert(entry.getTerminator() && "Entry block must have a terminator!");
    AllocaInst* alloca =
      new AllocaInst(storageTy, NULL, "local"+utostr(i), entry.getTerminator());
    it = slotMap.insert(it, std::make_pair(storageTy, alloca));
  }

  new StoreInst(value, it->second, insertAtEnd);
}

void Locals::store(unsigned i, Value* value, Instruction* insertBefore)
{
  const Type* valueTy = value->getType();
  const Type* storageTy = getStorageType(valueTy);
  if (valueTy != storageTy)
    value = new CastInst(value, storageTy, "to-storage-type", insertBefore);

  SlotMap& slotMap = TheLocals[i];
  SlotMap::iterator it = slotMap.find(storageTy);

  if (it == slotMap.end()) {
    // Insert the alloca at the beginning of the entry block.
    BasicBlock& entry = insertBefore->getParent()->getParent()->getEntryBlock();
    assert(entry.getTerminator() && "Entry block must have a terminator!");
    AllocaInst* alloca =
      new AllocaInst(storageTy, NULL, "local"+utostr(i), entry.getTerminator());
    it = slotMap.insert(it, std::make_pair(storageTy, alloca));
  }

  new StoreInst(value, it->second, insertBefore);
}

llvm::Value* Locals::load(unsigned i, const Type* valueTy,
                          BasicBlock* insertAtEnd)
{
  const Type* storageTy = getStorageType(valueTy);

  SlotMap& slotMap = TheLocals[i];
  SlotMap::iterator it = slotMap.find(storageTy);

  assert(it != slotMap.end() && "Attempt to load a non initialized global!");
  Value* value = new LoadInst(it->second, "load", insertAtEnd);
  if (valueTy != storageTy)
    value = new CastInst(value, valueTy, "from-storage-type", insertAtEnd);
  return value;
}
