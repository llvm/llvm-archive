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
#include <map>
#include <vector>

namespace llvm {

  class AllocaInst;

} // namespace llvm

namespace llvm {  namespace Java {

  class OperandStack {
    unsigned currentDepth;
    typedef std::map<const Type*, AllocaInst*> SlotMap;
    std::vector<SlotMap> TheStack;

  public:
    explicit OperandStack(unsigned maxDepth)
      : currentDepth(0), TheStack(maxDepth) { }

    unsigned getDepth() const { return currentDepth; }
    void setDepth(unsigned newDepth) {
      assert(newDepth < TheStack.size() &&
             "Cannot set depth greater than the max depth!");
      currentDepth = newDepth;
    }

    /// @brief - Pushes the value \c value on the virtual operand
    /// stack and appends any instructions to implement this to \c
    /// insertAtEnd BasicBlock
    void push(Value* value, BasicBlock* insertAtEnd);

    /// @brief - Pops a value of type \c type from the virtual operand
    /// stack and appends any instructions to implement this to \c
    /// insertAtEnd BasicBlock
    Value* pop(const Type* type, BasicBlock* insertAtEnd);

    void do_pop(BasicBlock* insertAtEnd);
    void do_pop2(BasicBlock* insertAtEnd);
    void do_dup(BasicBlock* insertAtEnd);
    void do_dup_x1(BasicBlock* insertAtEnd);
    void do_dup_x2(BasicBlock* insertAtEnd);
    void do_dup2(BasicBlock* insertAtEnd);
    void do_dup2_x1(BasicBlock* insertAtEnd);
    void do_dup2_x2(BasicBlock* insertAtEnd);
    void do_swap(BasicBlock* insertAtEnd);

  private:
    AllocaInst* getOrCreateSlot(SlotMap& slotMap,
                                const Type* type,
                                BasicBlock* bb);
    void copySlots(const SlotMap& src, SlotMap& dst, BasicBlock* insertAtEnd);
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_OPERANDSTACK_H
