//===-- Support.h - Support functions ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains support functions for the compiler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_SUPPORT_H
#define LLVM_JAVA_SUPPORT_H

#include <llvm/Type.h>
#include <llvm/Java/Compiler.h>

namespace llvm {  namespace Java {

  inline const Type* getStorageType(const Type* type) {
    if (isa<PointerType>(type))
      return ObjectBaseRefTy;
    else if (type == Type::BoolTy ||
             type == Type::SByteTy ||
             type == Type::UShortTy ||
             type == Type::ShortTy)
      return Type::IntTy;
    else
      return type;
  }

  inline bool isTwoSlotType(const Type* type) {
    return type == Type::LongTy || type == Type::DoubleTy;
  }

  inline bool isOneSlotType(const Type* type) {
    return !isTwoSlotType(type);
  }

} } // namespace llvm::Java

#endif//LLVM_JAVA_SUPPORT_H
