//===- AddStubs.cpp - Add Stubs Pass --------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a stub adder pass. Because class2llvm is not able to
// compile all of java at the moment, this pass is used to add dummy returns
// to those functions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "addstubs"

#include <llvm/Pass.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include <llvm/Support/Debug.h>
#include <iostream>

using namespace llvm;

namespace {

  static Constant* ALL_ONES = ConstantUInt::getAllOnesValue(Type::ULongTy);

  struct AddStubs : public ModulePass {
    virtual bool runOnModule(Module &M) {
      for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F)
        if (F->empty() &&
            (F->getName().find("java") != std::string::npos ||
             F->getName().find("gnu") != std::string::npos)) {
          DEBUG(std::cerr << "Stubbing out: " << F->getName() << '\n');
          BasicBlock* entry = new BasicBlock("entry", F);
          if (F->getReturnType() == Type::VoidTy)
            new ReturnInst(NULL, entry);
          else
            new ReturnInst(
              new CastInst(ALL_ONES, F->getReturnType(), "dummy-value", entry),
              entry);
        }
      return true;
    }
  };
  RegisterOpt<AddStubs> X("addstubs", "Add Stubs pass");
}
