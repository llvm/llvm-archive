#ifndef INSERT_BOUNDS_H
#define INSERT_BOUNDS_H

#include "llvm/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "safecode/Config/config.h"
#include "ArrayBoundsCheck.h"
#include "ConvertUnsafeAllocas.h"

#ifndef LLVA_KERNEL
#include "SafeDynMemAlloc.h"
#include "poolalloc/PoolAllocate.h"
#endif

namespace llvm {

ModulePass *creatInsertPoolChecks();
using namespace CUA;

struct PreInsertPoolChecks : public ModulePass {
    friend struct InsertPoolChecks;
	private :
    // Flags whether we want to do dangling checks
    bool DanglingChecks;

	public :
    static char ID;
    PreInsertPoolChecks (bool DPChecks = false)
        : ModulePass ((intptr_t) &ID) {
      DanglingChecks = DPChecks;
    }
    const char *getPassName() const { return "Register Global variable into pools"; }
    virtual bool runOnModule(Module &M);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
#ifndef LLVA_KERNEL      
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<ArrayBoundsCheck>();
      AU.addRequired<EmbeCFreeRemoval>();
      AU.addRequired<TargetData>();
      AU.addPreserved<PoolAllocateGroup>();
#else 
      AU.addRequired<TDDataStructures>();
#endif
    };
    private :
#ifndef  LLVA_KERNEL
  PoolAllocateGroup * paPass;
  EmbeCFreeRemoval *efPass;
  TargetData * TD;
#else
  TDDataStructures * TDPass;
#endif  
  Constant *RuntimeInit;
  DSNode* getDSNode(const Value *V, Function *F);
  unsigned getDSNodeOffset(const Value *V, Function *F);
#ifndef LLVA_KERNEL  
  Value * getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed = true);
  void registerGlobalArraysWithGlobalPools(Module &M);
#endif  
};

struct InsertPoolChecks : public FunctionPass {
    public :
    static char ID;
    InsertPoolChecks () : FunctionPass ((intptr_t) &ID) { }
    const char *getPassName() const { return "Inserting Pool checks Pass"; }
    virtual bool doInitialization(Module &M);
    virtual bool doFinalization(Module &M);
    virtual bool runOnFunction(Function &F);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
//      AU.addRequired<CompleteBUDataStructures>();
//      AU.addRequired<TDDataStructures>();
#ifndef LLVA_KERNEL      
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<ArrayBoundsCheck>();
      AU.addRequired<EmbeCFreeRemoval>();
      AU.addRequired<TargetData>();
      AU.addPreserved<PoolAllocateGroup>();
      AU.addRequired<PreInsertPoolChecks>();
      AU.addPreserved<PreInsertPoolChecks>();
#else 
      AU.addRequired<TDDataStructures>();
#endif
    };
    private :
      ArrayBoundsCheck * abcPass;
#ifndef  LLVA_KERNEL
  PoolAllocateGroup * paPass;
  EmbeCFreeRemoval *efPass;
  TargetData * TD;
#else
  TDDataStructures * TDPass;
#endif  
  Constant *PoolCheck;
  Constant *PoolCheckUI;
  Constant *PoolCheckArray;
  Constant *PoolCheckArrayUI;
  Constant *ExactCheck;
  Constant *ExactCheck2;
  Constant *FunctionCheck;
  Constant *GetActualValue;
  Constant *StackFree;
  PreInsertPoolChecks * pipc;
  void addCheckProto(Module &M);
  void addPoolChecks(Function &F);
  void addGetElementPtrChecks(BasicBlock * BB);
  void addGetActualValue(llvm::ICmpInst*, unsigned int);
  bool insertExactCheck (GetElementPtrInst * GEP);
  bool insertExactCheck (Instruction * , Value *, Value *, Instruction *);
  DSNode* getDSNode(const Value *V, Function *F);
  unsigned getDSNodeOffset(const Value *V, Function *F);
  void addLoadStoreChecks(Function &F);
  void registerStackObjects (Module & M);
  void registerAllocaInst(AllocaInst *AI, AllocaInst *AIOrig);
  void addExactCheck (Value * P, Value * I, Value * B, Instruction * InsertPt);
  void addExactCheck2 (Value * B, Value * R, Value * C, Instruction * InsertPt);
  DSGraph & getDSGraph (Function & F);
#ifndef LLVA_KERNEL  
  void addLSChecks(Value *Vnew, const Value *V, Instruction *I, Function *F);
  Value * getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed = true);
  void registerGlobalArraysWithGlobalPools(Module &M);
#else
  void addLSChecks(Value *V, Instruction *I, Function *F);
  Value * getPoolHandle(const Value *V, Function *F);
#endif  
};

/// Monotonic Loop Optimization
struct MonotonicLoopOpt : public LoopPass {
  static char ID;
  const char *getPassName() const { return "Optimize SAFECode checkings in monotonic loops"; }
  MonotonicLoopOpt() : LoopPass((intptr_t) &ID) {}
  virtual bool doInitialization(Loop *L, LPPassManager &LPM); 
  virtual bool doFinalization(); 
  virtual bool runOnLoop(Loop *L, LPPassManager &LPM);
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfo>();
    AU.addRequired<ScalarEvolution>();
  }
  private:
  LoopInfo * LI;
  ScalarEvolution * scevPass;
  bool isMonotonicLoop(Loop * L, Value * loopVar);
  bool isHoistableGEP(GetElementPtrInst * GEP, Loop * L);
  void insertEdgeBoundsCheck(int checkFunctionId, Loop * L, const CallInst * callInst, GetElementPtrInst * origGEP, Instruction *
  ptIns, int type);
  bool optimizeCheck(Loop *L);
};
}
#endif
