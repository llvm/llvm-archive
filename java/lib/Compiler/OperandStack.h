//===-- OperandStack.h - Java operand stack ---------------------*- C++ -*-===//
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

#ifndef LLVM_JAVA_OPERANDSTACK_H
#define LLVM_JAVA_OPERANDSTACK_H

#include <llvm/Value.h>
#include <llvm/Instruction.h>
#include <llvm/Java/Bytecode.h>

#include <stack>

namespace llvm {

  class AllocaInst;

} // namespace llvm

namespace llvm {  namespace Java {

  class OperandStack {
    std::stack<AllocaInst*, std::vector<AllocaInst*> > TheStack;

  public:
    /// @brief - Pushes the value \c value on the virtual operand
    /// stack and appends any instructions to implement this to \c
    /// insertAtEnd BasicBlock
    void push(Value* value, BasicBlock* insertAtEnd);

    /// @brief - Pops a value from the virtual operand stack and
    /// appends any instructions to implement this to \c insertAtEnd
    /// BasicBlock
    Value* pop(BasicBlock* insertAtEnd);
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_OPERANDSTACK_H
