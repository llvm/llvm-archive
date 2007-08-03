#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/Statistic.h"

#include <set>
#include <vector>
#include <algorithm>
using namespace llvm;

namespace {

  Statistic<> FuncAdded("CSCloner", "Number of functions added");
  Statistic<> IndDirSplit("CSCloner", "Number of direct and indirect splits");
  Statistic<> LeafClone("CSCloner", "Number of leaves cloned");
  Statistic<> ShallowClone("CSCloner", "Number of shallow functions cloned");
  Statistic<> ConstantClone("CSCloner", "Number of functions with constant pointers cloned");

  class CSCloner : public ModulePass {

    bool isLeaf(Function* F) {
      for (Function::iterator FI = F->begin(), FE = F->end();
           FI != FE; ++FI)
        for (BasicBlock::iterator BI = FI->begin(), BE = FI->end();
             BI != BE; ++BI)
          if(isa<CallInst>(BI) || isa<InvokeInst>(BI))
            return false;
      return true;
    }

    bool isLevelOne(Function* F) {
      for (Function::iterator FI = F->begin(), FE = F->end();
           FI != FE; ++FI)
        for (BasicBlock::iterator BI = FI->begin(), BE = FI->end();
             BI != BE; ++BI) {
          if(LoadInst* LI = dyn_cast<LoadInst>(BI))
            if (isa<PointerType>(LI->getType()))
              return false;
          if(StoreInst* SI = dyn_cast<StoreInst>(BI)) 
            if (isa<PointerType>(SI->getOperand(0)->getType()))
              return false;
        }
      return true;
    }

    bool isLevelTwo(Function* F) {
      for (Function::iterator FI = F->begin(), FE = F->end();
           FI != FE; ++FI)
        for (BasicBlock::iterator BI = FI->begin(), BE = FI->end();
             BI != BE; ++BI) {
          if(LoadInst* LI = dyn_cast<LoadInst>(BI))
            if (isa<PointerType>(LI->getType()))
              for (Value::use_iterator ii = LI->use_begin(), ee = LI->use_end();
                   ii != ee; ++ii)
                if (isa<LoadInst>(ii))
                  return false;
        }
      return true;
    }

    Function* clone(Function* F) {
      Function* FNew = CloneFunction(F);
      FNew->setLinkage(Function::InternalLinkage);
      ++FuncAdded;
      F->getParent()->getFunctionList().push_back(FNew);
      return FNew;
    }

    bool isDirect(Function* F) {
      for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
           ii != ee; ++ii) {
        CallInst* CI = dyn_cast<CallInst>(*ii);
        if (CI && CI->getCalledFunction() == F)
          return true;
      }
      return false;
    }

    bool isUnknown(Function* F) {
          for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
               ii != ee; ++ii) {
            CallInst* CI = dyn_cast<CallInst>(*ii);
            if (!CI || CI->getCalledFunction() != F)
              return true;
          }
          return false;
    }

    bool hasPointer(Function* F) {
      const FunctionType* FT = F->getFunctionType();
      if (FT->isVarArg() || isa<PointerType>(FT->getReturnType()))
        return true;
      else
        for (FunctionType::param_iterator pi = FT->param_begin(), pe = FT->param_end();
             pi != pe; ++pi)
          if (isa<PointerType>(pi->get()))
            return true;
      return false;
    }    

    bool hasConstArgs(CallInst* CI) {
      for (unsigned x = 1; x < CI->getNumOperands(); ++x)
        if (isa<Constant>(CI->getOperand(x)) && isa<PointerType>(CI->getOperand(x)->getType()))
          return true;
      return false;
    }

    bool hasConstArgs(Function* F) {
      for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
           ii != ee; ++ii) {
        CallInst* CI = dyn_cast<CallInst>(*ii);
        if (CI && CI->getCalledFunction() == F)
          if (hasConstArgs(CI))
            return true;
      }
      return false;
    }


  public:
    bool runOnModuleI(Module& M) {
      
      bool changed = false;

      //first figure out how functions are used
      std::set<Function*> DirectCalls;
      std::set<Function*> Unknowns;
      std::set<Function*> TakesPointers;
      std::set<Function*> Leaf;
      std::set<Function*> Shallow;
      std::set<Function*> ConstantArgs;
      std::vector<std::string> IgnoreList;
      IgnoreList.push_back("kmalloc");
      IgnoreList.push_back("__vmalloc");
      IgnoreList.push_back("kmem_cache_alloc");
      IgnoreList.push_back("__alloc_bootmem");
      IgnoreList.push_back(" __get_free_pages");
      IgnoreList.push_back("kfree");
      IgnoreList.push_back("vfree");
      IgnoreList.push_back("free_pages");

      for (Module::iterator MI = M.begin(), ME = M.end();
           MI != ME; ++MI)
        if (!MI->isExternal() &&
            std::find(IgnoreList.begin(), IgnoreList.end(), MI->getName()) == IgnoreList.end()) {
          if (isLeaf(MI))
            Leaf.insert(MI);
          if (isLevelOne(MI) || isLevelTwo(MI))
            Shallow.insert(MI);
          if (isDirect(MI))
            DirectCalls.insert(MI);
          if (isUnknown(MI))
            Unknowns.insert(MI);
          if (hasPointer(MI))
            TakesPointers.insert(MI);
          if (hasConstArgs(MI))
            ConstantArgs.insert(MI);
        }
      
      //now think about replicating some functions
      for (Module::iterator MI = M.begin(), ME = M.end();
           MI != ME; ++MI) {
        if(TakesPointers.find(MI) != TakesPointers.end()) {

          //if something is used for both direct calls and indirect calls,
          //clone a function for the indirect calls
          if (DirectCalls.find(MI) != DirectCalls.end() &&
              Unknowns.find(MI) != Unknowns.end()) {
            changed = true;
            ++IndDirSplit;
            Function* FNew = clone(MI);
            for (Value::use_iterator ii = MI->use_begin(), ee = MI->use_end();
                 ii != ee; ++ii) {
              CallInst* CI = dyn_cast<CallInst>(*ii);
              if (CI && CI->getCalledFunction() == MI) {
                CI->setOperand(0, FNew);
              }
            }
          }
          
          //if it takes constants in pointer parameters
          if (ConstantArgs.find(MI) != ConstantArgs.end() && !MI->hasOneUse() && !MI->use_empty()) {
            for (Value::use_iterator ii = MI->use_begin(), ee = MI->use_end();
                 ii != ee; ++ii) {
              CallInst* CI = dyn_cast<CallInst>(*ii);
              if (CI && CI->getCalledFunction() == MI && hasConstArgs(CI)) {
                Function* FNew = clone(MI);
                ++ConstantClone;
                changed = true;
                CI->setOperand(0, FNew);
              }
            }
          }
          
          //if it is a leaf, clone it
          if (Leaf.find(MI) != Leaf.end() && !MI->hasOneUse() && !MI->use_empty()) {
            for (Value::use_iterator ii = MI->use_begin(), ee = MI->use_end();
                 ii != ee; ++ii) {
              CallInst* CI = dyn_cast<CallInst>(*ii);
              if (CI && CI->getCalledFunction() == MI) {
                Function* FNew = clone(MI);
                ++LeafClone;
                changed = true;
                CI->setOperand(0, FNew);
              }
            }
          }
          
          //if it has only level 1 loads (aka no loads of pointers), clone it
          if (Shallow.find(MI) != Shallow.end() && !MI->hasOneUse() && !MI->use_empty()) {
            for (Value::use_iterator ii = MI->use_begin(), ee = MI->use_end();
                 ii != ee; ++ii) {
              CallInst* CI = dyn_cast<CallInst>(*ii);
              if (CI && CI->getCalledFunction() == MI) {
                Function* FNew = clone(MI);
                ++ShallowClone;
                changed = true;
                CI->setOperand(0, FNew);
              }
            }
          }
        }
      }
      return changed;
    }

    virtual bool runOnModule(Module& M) {
      //      int x = 4;
      //      while (runOnModuleI(M) && --x) {}
      //      return true;
      return runOnModuleI(M);
    }
  };

  RegisterPass<CSCloner> X("csclone", "Cloning for Context Sensitivity");

}
