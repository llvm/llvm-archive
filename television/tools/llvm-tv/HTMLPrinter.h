#ifndef HTMLPRINTER_H
#define HTMLPRINTER_H

#include "HTMLMarkup.h"
#include "llvm/Assembly/CachedWriter.h"
#include "llvm/Support/InstVisitor.h"
#include <map>
#include <ostream>

namespace llvm {
  class Module;
  class Function;
  class BasicBlock;

  class Instruction;
  class BranchInst;
  class PHINode;
  class ReturnInst;
  class SwitchInst;
  class BinaryOperator;
  class InvokeInst;
  class UnwindInst;
  class SetCondInst;
  class AllocationInst;
  class FreeInst;
  class StoreInst;
  class GetElementPtrInst;
  class CastInst;
  class SelectInst;
  class ShiftInst;
  class VANextInst;
  class VAArgInst;

  class Type;

  class SlotCalculator;
}

class HTMLPrinter : public llvm::InstVisitor<HTMLPrinter> {
private:
  llvm::CachedWriter &cw;
  std::ostream &os;
  HTMLMarkup &html;
  std::map<const llvm::Type *, std::string> TypeNames;
  llvm::SlotCalculator *Table;

  void writeOperand(const llvm::Value *Operand, bool PrintType, 
                    bool PrintName=true);
  void printFunctionHeader(llvm::Function &F);
  void printType(const llvm::Type *Ty);
  void printName(llvm::Instruction &I);
  void printOperands(llvm::Instruction &I);
public:
  HTMLPrinter(llvm::CachedWriter &CW, HTMLMarkup &HM) 
    : cw(CW), os(CW.getStream()), html(HM) {}

  using llvm::InstVisitor<HTMLPrinter>::visit;

  // Program structure
  void visit(llvm::Module &M);
  void visit(llvm::GlobalVariable &GV);
  void visit(llvm::Function &F);
  void visitBasicBlock(llvm::BasicBlock &BB);

  // Instructions
  void visitInstruction(llvm::Instruction &LI);
  void visitSwitchInst(llvm::SwitchInst &SI);
  void visitCallInst(llvm::CallInst &CI);
  void visitPHINode(llvm::PHINode &PN);
  void visitInvokeInst(llvm::InvokeInst &I);
  void visitAllocationInst(llvm::AllocationInst &I);
  void visitCastInst(llvm::CastInst &I);
  void visitVANextInst(llvm::VANextInst &I);
  void visitVAArgInst(llvm::VAArgInst &I);
};

#endif
