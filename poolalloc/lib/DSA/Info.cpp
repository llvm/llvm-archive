#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstVisitor.h"

#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"

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

struct dsstat {
  int U;
  int I;
  int UI;
  int F;
  int T;
  void add(DSNode* N) {
    if (!N) return;
    if (N->isIncomplete()) ++I;
    if (N->isUnknownNode()) ++U;
    if (N->isIncomplete() && N->isUnknownNode()) ++UI;
    if (N->isNodeCompletelyFolded()) ++F;
    ++T;
  }
  void print(const char* name) const {
    std::cerr << name << ",  U: " <<  U << "\n";
    std::cerr << name << ",  I: " <<  I << "\n";
    std::cerr << name << ", UI: " << UI << "\n";
    std::cerr << name << ",  F: " <<  F << "\n";
    std::cerr << name << ",  T: " <<  T << "\n";
  }
  void clear() {
    U = I = UI = F = T = 0;
  }
  void addNorm(struct dsstat& o) {
    if (o.U) ++U;
    if (o.I) ++I;
    if (o.UI) ++UI;
    if (o.F) ++F;
    if (o.T) ++T;
  }
};

namespace {
  class LDSTCount : public FunctionPass, public InstVisitor<LDSTCount> {
    TDDataStructures* T;
    DSGraph* G;

    struct dsstat LD;
    struct dsstat ST;
    struct dsstat GEP;
    struct dsstat GEPA;
    struct dsstat ALL;
    struct dsstat LD_F;
    struct dsstat ST_F;
    struct dsstat GEP_F;
    struct dsstat GEPA_F;
    struct dsstat ALL_F;
    struct dsstat LD_T;
    struct dsstat ST_T;
    struct dsstat GEP_T;
    struct dsstat GEPA_T;
    struct dsstat ALL_T;


  public:
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<TDDataStructures>();
    }

    bool doInitialization(Module &M) {
      LD.clear();
      ST.clear();
      GEP.clear();
      GEPA.clear();
      ALL.clear();
      LD_F.clear();
      ST_F.clear();
      GEP_F.clear();
      GEPA_F.clear();
      ALL_F.clear();
      LD_T.clear();
      ST_T.clear();
      GEP_T.clear();
      GEPA_T.clear();
      ALL_T.clear();
      return false;
    }

    bool doFinalization (Module &M) {
      LD.print("Loads");
      ST.print("Stores");
      GEP.print("GEPs");
      GEPA.print("Array GEPs");
      ALL.print("All");
      LD_F.print("Loads, Fn");
      ST_F.print("Stores, Fn");
      GEP_F.print("GEPs, Fn");
      GEPA_F.print("GEPs, Fn");
      ALL_F.print("All, Fn");
      return false;
    }

    bool runOnFunction(Function& F) {
      T = &getAnalysis<TDDataStructures>();
      G = &T->getDSGraph(F);
      visit(F);
      LD_F.addNorm(LD_T);
      ST_F.addNorm(ST_T);
      GEP_F.addNorm(GEP_T);
      GEPA_F.addNorm(GEPA_T);
      ALL_F.addNorm(ALL_T);
      LD_T.clear();
      ST_T.clear();
      GEP_T.clear();
      GEPA_T.clear();
      ALL_T.clear();
      return false;
    }

    bool hasAllConstantIndices(GetElementPtrInst* G) const {
      for (unsigned i = 1, e = G->getNumOperands(); i != e; ++i) {
        if (!isa<ConstantInt>(G->getOperand(i)))
          return false;
      }
      return true;
    }

    void visitGetElementPtrInst(User& VGEP) {
      DSNode* N = G->getNodeForValue(VGEP.getOperand(0)).getNode();
      if (hasAllConstantIndices(cast<GetElementPtrInst>(&VGEP))) {
        GEP.add(N);
        GEP_T.add(N);
      } else {
        GEPA.add(N);
        GEPA_T.add(N);
      }
      ALL.add(N);
      ALL_T.add(N);
    }

    void visitLoadInst(LoadInst &LI) {
      DSNode* N = G->getNodeForValue(LI.getOperand(0)).getNode();
      LD.add(N);
      LD_T.add(N);
      ALL.add(N);
      ALL_T.add(N);
    }

    void visitStoreInst(StoreInst &SI) {
      DSNode* N = G->getNodeForValue(SI.getOperand(1)).getNode();
      ST.add(N);
      ST_T.add(N);
      ALL.add(N);
      ALL_T.add(N);
    }
  };

  RegisterPass<LDSTCount> Y("mem-info", "DSA memory info pass");

}
