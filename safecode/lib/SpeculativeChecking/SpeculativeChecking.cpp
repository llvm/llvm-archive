/// This file define a pass to lower synchronous checking calls to
/// speculative checking calls

#include <iostream>
#include <set>
#include <map>
#include "llvm/Module.h"
#include "safecode/Config/config.h"
#include "SpeculativeChecking.h"
#include "VectorListHelper.h"

using namespace llvm;
char SpeculativeCheckingPass::ID = 0;

static RegisterPass<SpeculativeCheckingPass> passSpeculativeChecking ("speculative-checking", "Lower checkings to speculative checkings");

/// Static Members
namespace {
  typedef std::map<Function *, Function *> CheckFuncMapTy;
  CheckFuncMapTy sCheckFuncMap;
  Constant * sFuncWaitForSyncToken;
}

namespace llvm {
  ////////////////////////////////////////////////////////////////////////////
  // SpeculativeChecking Methods
  ////////////////////////////////////////////////////////////////////////////

  bool
  SpeculativeCheckingPass::doInitialization(Module & M) {
    static const Type * VoidTy = Type::VoidTy;
    static const Type * vpTy = PointerType::getUnqual(Type::Int8Ty);

    #define REG_FUNC(name, ...) do {					\
	Function * funcOrig = dyn_cast<Function>(M.getOrInsertFunction(name, FunctionType::get(VoidTy, args<const Type*>::list(__VA_ARGS__), false))); \
	Function * funcSpec = dyn_cast<Function>(M.getOrInsertFunction("__sc_" name, FunctionType::get(VoidTy, args<const Type*>::list(__VA_ARGS__), false))); \
      sCheckFuncMap[funcOrig] = funcSpec;				\
    } while (0)

    REG_FUNC ("poolcheck",     vpTy, vpTy);
    REG_FUNC ("poolcheckui",   vpTy, vpTy);
    REG_FUNC ("boundscheck",   vpTy, vpTy, vpTy);
    REG_FUNC ("boundscheckui", vpTy, vpTy, vpTy);

#undef REG_FUNC

    sFuncWaitForSyncToken = M.getOrInsertFunction("__sc_wait_for_completion", FunctionType::get(VoidTy, args<const Type*>::list(), false));
    return true;
  }

  bool
  SpeculativeCheckingPass::runOnBasicBlock(BasicBlock & BB) {
    bool changed = false;
    std::set<CallInst *> toBeRemoved;
    for (BasicBlock::iterator I = BB.begin(); I != BB.end(); ++I) {
      if (CallInst * CI = dyn_cast<CallInst>(I)) {
        bool ret = lowerCall(CI);
	if (ret) {
	  toBeRemoved.insert(CI);
	}
	changed |= ret;
      }
    }

    for (std::set<CallInst *>::iterator it = toBeRemoved.begin(), e = toBeRemoved.end(); it != e; ++it) {
      (*it)->eraseFromParent();
    }

    return changed;
  }

  bool
  SpeculativeCheckingPass::lowerCall(CallInst * CI) {
    Function *F = CI->getCalledFunction();
    if (!F) return false;
    CheckFuncMapTy::iterator it = sCheckFuncMap.find(F);
    if (it == sCheckFuncMap.end()) return false;

    BasicBlock::iterator ptIns(CI);
    ++ptIns;
    std::vector<Value *> args;
    for (unsigned i = 1; i < CI->getNumOperands(); ++i) {
      args.push_back(CI->getOperand(i));
    }

    CallInst::Create(it->second, args.begin(), args.end(), "", ptIns);
    CallInst::Create(sFuncWaitForSyncToken, "", ptIns);
    return true;
  }
}
