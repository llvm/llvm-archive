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

#include "Resolver.h"
#include <llvm/Value.h>
#include <llvm/Instruction.h>
#include <map>
#include <vector>

namespace llvm {

  class AllocaInst;
  class Instruction;
  class Value;

} // namespace llvm

namespace llvm {  namespace Java {

  class OperandStack {
    const Resolver* resolver_;
    unsigned currentDepth_;
    typedef std::map<const Type*, AllocaInst*> SlotMap;
    std::vector<SlotMap> stack_;

  public:
    explicit OperandStack(const Resolver& resolver, unsigned maxDepth)
      : resolver_(&resolver),
        currentDepth_(0),
        stack_(maxDepth) { }

    unsigned getDepth() const { return currentDepth_; }
    void setDepth(unsigned newDepth) {
      assert(newDepth < stack_.size() &&
             "Cannot set depth greater than the max depth!");
      currentDepth_ = newDepth;
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
