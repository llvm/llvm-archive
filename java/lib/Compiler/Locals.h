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

#ifndef LLVM_JAVA_LOCALS_H
#define LLVM_JAVA_LOCALS_H

#include <llvm/Java/Bytecode.h>
#include <vector>

namespace llvm {

  class AllocaInst;
  class BasicBlock;
  class Value;

} // namespace llvm

namespace llvm { namespace Java {

  class Locals {
    std::vector<AllocaInst*> TheLocals;

  public:
    explicit Locals(unsigned maxLocals);

    /// @brief - Stores the value \c value on the \c i'th local
    /// variable and appends any instructions to implement this to \c
    /// insertAtEnd BasicBlock
    void store(unsigned i, Value* value, BasicBlock* insertAtEnd);

    /// @brief - Loads the value of the \c i'th local variable and
    /// appends any instructions to implement this to \c insertAtEnd
    /// BasicBlock
    Value* load(unsigned i, BasicBlock* insertAtEnd);
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_LOCALS_H
