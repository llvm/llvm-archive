#include <map>
#include <set>
#include <iterator>
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"

using namespace llvm;

namespace {
  struct ProofStrip : public ModulePass {
    const Value* getProof(const Value* V) {
      return stuff[V];
    }

    virtual bool runOnModule(Module &M);

    private:
    void setProof(const Value* V, const Value* P) {
      stuff[V] = P;
    }

    std::map<const Value*, const Value*> stuff;
  };



  // call AddProof until you are done annotating
  // then call Finalize
  struct ProofPlace {
    std::map<Value*, Value*> stuff;

    void AddProof(Value* V, Value* P) {
      assert(isa<PointerType>(V->getType()));
      assert(isa<ConstantInt>(P));
      stuff[V] = P;
    }
    
    void AddProof(Value* V, unsigned P) {
      AddProof(V, ConstantInt::get(Type::LongTy,P));
    }

    void Finalize(Module& M) {
      //try to be semi clever
      //find homes for values
      BasicBlock* SomeBB;
      std::map<std::pair<BasicBlock*, Value*>, std::set<Value*> > homes;
      for (std::map<Value*, Value*>::iterator ii = stuff.begin(),
             ee = stuff.end(); ii != ee; ++ii) {
        Value* V = ii->first;
        if (isa<GlobalValue>(V)) {
          homes[std::make_pair((BasicBlock*)0, ii->second)].insert(V);
        } else if (isa<Instruction>(V)) {
          homes[std::make_pair(cast<Instruction>(V)->getParent(), ii->second)].insert(V);
          SomeBB = cast<Instruction>(V)->getParent()
        } else {
          V->dump();
          assert(0 && "Not supported");
        }
      }

      //get proof function
      std::vector<const Type * > FTV;
      FTV.push_back(Type::LongTy);
      FunctionType* FT = FunctionType::get(Type::VoidTy, FTV, true);
      Function* F = M.getOrInsertFunction("llvm.proof.ptr", FT);
      
      //insert at end of BBs
      for (std::map<std::pair<BasicBlock*, Value*>, std::set<Value*> >::iterator
             ii = homes.begin(), ee = homes.end(); ii != ee; ++ii) {
        BasicBlock* BB = ii->first.first ? ii->first.first : SomeBB;
        std::vector<Value*> Args;
        Args.push_back(ii->first.second);
        std::copy(ii->second.begin(), ii->second.end(), std::back_insert_iterator<std::vector<Value*> > (Args));
        new CallInst(F, Args, "", BB->getTerminator());
      }
    }
  };
}
