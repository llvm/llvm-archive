#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/ADT/Statistic.h"

using namespace llvm;

Statistic<> Direct("calltarget", "Number of direct calls");
Statistic<> Indirect("calltarget", "Number of indirect calls");


namespace {
  struct CallInfo : public FunctionPass {
    virtual bool runOnFunction(Function &F) {
      for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI)
        for (BasicBlock::iterator II = BI->begin(), IE = BI->end(); II != IE; ++II)
          if (CallInst* CI = dyn_cast<CallInst>(&*II))
            if (CI->getCalledFunction())
              ++Direct;
            else
              ++Indirect;
      return false;
    }
  };
  
  RegisterPass<CallInfo> X("call-info", "Call Info Pass");
}
