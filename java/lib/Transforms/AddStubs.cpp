//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"

using namespace llvm;

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct StubAdder : public ModulePass {
    virtual bool runOnModule(Module &M) {
      for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F)
        if (F->empty() && F->getName().find("java") != std::string::npos) {
          BasicBlock* entry = new BasicBlock("entry", F);
          if (F->getReturnType() == Type::VoidTy)
            new ReturnInst(NULL, entry);
          else
            new ReturnInst(UndefValue::get(F->getReturnType()), entry);
        }
      return true;
    }
  };
  RegisterOpt<StubAdder> X("stubadder", "Stub Adder Pass");
}
