#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/Value.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include <iostream>
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "llvm/Support/CallSite.h"

using namespace llvm;

namespace {
  class KPCount : public ModulePass {
  public:
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<TDDataStructures>();
    }
    bool runOnModule(Module& M) {
      TDDataStructures* T = &getAnalysis<TDDataStructures>();
      Function* f = M.getNamedFunction("kmem_cache_alloc");
      for (Value::use_iterator ii = f->use_begin(), ee = f->use_end();
	   ii != ee; ++ii) {
	if (CallInst* CI = dyn_cast<CallInst>(*ii)) {
	  if (LoadInst* LI = dyn_cast<LoadInst>(CI->getOperand(1))) {
	    CallSite cs = CallSite::get(CI);
	    DSNode* N = T->getDSGraph(*cs.getCaller())
	      .getNodeForValue(CI).getNode();
	    if (N->isNodeCompletelyFolded()) 
	      std::cerr << "F ";
	    else
	      std::cerr << "S ";
	    std::cerr << LI->getOperand(0)->getName() << "\n";
	  }
	}
      }
      return false;
    }
  };

  RegisterPass<KPCount> X("kpcount", "Count Kernel Pool Thingies");
}
