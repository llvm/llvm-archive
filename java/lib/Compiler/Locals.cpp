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
#include <llvm/BasicBlock.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Java/Compiler.h>

using namespace llvm::Java;

Locals::Locals(unsigned maxLocals)
  : TheLocals(maxLocals)
{

}

void Locals::store(unsigned i, Value* value, BasicBlock* insertAtEnd)
{
  const Type* valueTy = value->getType();
  // All pointer types are cast to a pointer to
  // llvm_java_lang_object_base.
  if (isa<PointerType>(valueTy))
    value = new CastInst(value, ObjectBaseRefTy,
                         "to-object-base", insertAtEnd);

  // Values of jboolean, jbyte, jchar and jshort are extended to a
  // jint when stored in a local slot.
  else if (valueTy == Type::BoolTy ||
	   valueTy == Type::SByteTy ||
	   valueTy == Type::UShortTy ||
	   valueTy == Type::ShortTy)
    value = new CastInst(value, Type::IntTy, "int-extend", insertAtEnd);

  valueTy = value->getType();

  SlotMap& slotMap = TheLocals[i];
  SlotMap::iterator it = slotMap.find(valueTy);

  if (it == slotMap.end()) {
    // Insert the alloca at the beginning of the entry block.
    BasicBlock* entry = &insertAtEnd->getParent()->getEntryBlock();
    AllocaInst* alloca;
    if (entry->empty())
      alloca = new AllocaInst(
        value->getType(),
        NULL,
        "local" + utostr(i),
        entry);
    else
      alloca = new AllocaInst(
        value->getType(),
        NULL,
        "local" + utostr(i),
        &entry->front());
    it = slotMap.insert(it, std::make_pair(valueTy, alloca));
  }

  new StoreInst(value, it->second, insertAtEnd);
}

llvm::Value* Locals::load(unsigned i, const Type* type, BasicBlock* insertAtEnd)
{
  SlotMap& slotMap = TheLocals[i];
  SlotMap::iterator it = slotMap.find(type);

  assert(it != slotMap.end() && "Attempt to load a non initialized global!");
  return new LoadInst(it->second, "load", insertAtEnd);
}
