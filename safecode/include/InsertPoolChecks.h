#ifndef INSERT_BOUNDS_H
#define INSERT_BOUNDS_H

#include "safecode/Config/config.h"
#include "ConvertUnsafeAllocas.h"
#include "llvm/Pass.h"

#ifndef LLVA_KERNEL
#include "SafeDynMemAlloc.h"
#include "poolalloc/PoolAllocate.h"
#endif

#include <map>

namespace llvm {

ModulePass *creatInsertPoolChecks();
using namespace CUA;

struct InsertPoolChecks : public ModulePass {
    public :
    const char *getPassName() const { return "Inserting pool checks for array bounds "; }
    virtual bool runOnModule(Module &M);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<ConvertUnsafeAllocas>();
//      AU.addRequired<CompleteBUDataStructures>();
//      AU.addRequired<TDDataStructures>();
#ifndef LLVA_KERNEL      
      AU.addRequired<EquivClassGraphs>();
      AU.addRequired<PoolAllocate>();
      AU.addRequired<EmbeCFreeRemoval>();
      AU.addRequired<TargetData>();
#else 
      AU.addRequired<TDDataStructures>();
      AU.addRequired<TargetData>();
#endif
      
    };
    private :
    CUA::ConvertUnsafeAllocas * cuaPass;
  TargetData * TD;
#ifndef  LLVA_KERNEL
  PoolAllocate * paPass;
  EquivClassGraphs *equivPass;
  EmbeCFreeRemoval *efPass;
#else
  TDDataStructures * TDPass;
#endif  
  Function *PoolCheck;
  Function *PoolCheckArray;
  Function *PoolCheckIArray;
  Function *ExactCheck;
  Function *FunctionCheck;
  Function *FunctionCheckT;
  Function *FunctionCheckG;
  Function *BoundsCheck;
  Function *UIBoundsCheck;
  Function *getBounds;
  Function *UIgetBounds;
  Function *ExactCheck2;
  Function *ExactCheck3;
  Function *GetActualValue;
  Function *PoolRegister;
  Function *StackRegister;
  Function *ObjFree;
  Function *StackFree;
  Function *FuncRegister;
  Function *PoolRegMP;
  Function *PoolFindMP;
  Function *getBegin;
  Function *getEnd;

  // Map of DSNode to Function List Global Variables
  std::map<Value *, GlobalVariable *> FuncListMap;

  void simplifyGEPList();
  void addObjFrees(Module& M);
  void addMetaPools(Module& M, MetaPool* MP, DSNode* N);
  void addPoolCheckProto(Module &M);
  void addPoolChecks(Module &M);
  void addGetElementPtrChecks(Module &M);
  DSNode* getDSNode(const Value *V, Function *F);
  unsigned getDSNodeOffset(const Value *V, Function *F);
  void addLoadStoreChecks(Module &M);
  void TransformFunction(Function &F);
  void handleCallInst(CallInst *CI);
  void handleGetElementPtr(GetElementPtrInst *MAI);
  void addGetActualValue(SetCondInst *SCI, unsigned operand);
  void registerAllocaInst(AllocaInst *AI, AllocaInst *AIOrig);
  void registerGlobalArraysWithGlobalPools(Module &M);
  GlobalVariable * getOrCreateFunctionTable (DSNode * Node, Value * PH, Module * M);
  void addExactCheck  (Instruction * GEP, Value * Index, Value * Bound);
  void addExactCheck  (Value * Pointer, Value * Index, Value * Bound, Instruction * InsertPt);
  void addExactCheck2 (Value * Base, Value * Result, Value * Bound, Instruction * InsertPt);
  Value * addExactCheck3 (Value * Source, Value * Result, Value * Bound, Instruction * Next);
  bool insertExactCheck (GetElementPtrInst * GEP);
  bool insertExactCheck (Instruction * , Value *, Value *, Instruction *);
  void insertFunctionCheck(CallInst* CI);
  Value * insertBoundsCheck (Instruction * , Value *, Value *, Instruction *);
  bool AggregateGEPs (GetElementPtrInst * GEP, std::set<Instruction *> & GEPs);

#ifndef LLVA_KERNEL  
  void addLSChecks(Value *Vnew, const Value *V, Instruction *I, Function *F);
  Value * getPoolHandle(const Value *V, Function *F, PA::FuncInfo &FI, bool collapsed = false);
#else
  Value* getPD(DSNode* N, Module& M) { 
    if (!N) return 0;
    addMetaPools(M, N->getMP(), N);
    if (N->getMP())
      return N->getMP()->getMetaPoolValue();
    else
      return 0;
  }
  void addLSChecks(Value *V, Instruction *I, Function *F);
  Value * getPoolHandle(const Value *V, Function *F);
#endif  

};
}
#endif
