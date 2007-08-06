#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"

#include <set>
#include <map>
#include <vector>
#include <algorithm>
using namespace llvm;

namespace {

  Statistic<> FuncAdded("Cloner", "Number of functions added");
  Statistic<> IndDirSplit("IndCloner", "Number of direct and indirect splits");
  Statistic<> LeafClone("CSCloner", "Number of leaves cloned");
  Statistic<> ShallowClone("CSCloner", "Number of shallow functions cloned");
  Statistic<> ConstantClone("CSCloner", "Number of functions with constant pointers cloned");
  Statistic<> DepthClone("DepthCloner", "Number of functions cloned by depth");

  static cl::opt<int>
  CloneDepth("clone-depth", cl::Hidden, cl::init(4),
             cl::desc("depth to clone in the call graph"));

  static Function* clone(Function* F) {
    Function* FNew = CloneFunction(F);
    FNew->setLinkage(Function::InternalLinkage);
    ++FuncAdded;
    F->getParent()->getFunctionList().push_back(FNew);
    return FNew;
  }
  
  static bool hasPointer(Function* F) {
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
  
  static bool isDirect(Function* F) {
    for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
         ii != ee; ++ii) {
      CallInst* CI = dyn_cast<CallInst>(*ii);
      if (CI && CI->getCalledFunction() == F)
        return true;
    }
    return false;
  }
  
  static bool isUnknown(Function* F) {
    for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
         ii != ee; ++ii) {
      CallInst* CI = dyn_cast<CallInst>(*ii);
      if (!CI || CI->getCalledFunction() != F)
        return true;
    }
    return false;
  }
  
  static bool hasConstArgs(CallInst* CI) {
    for (unsigned x = 1; x < CI->getNumOperands(); ++x)
      if (isa<Constant>(CI->getOperand(x)) && isa<PointerType>(CI->getOperand(x)->getType()))
        return true;
    return false;
  }
  
  static bool hasConstArgs(Function* F) {
    for (Value::use_iterator ii = F->use_begin(), ee = F->use_end();
         ii != ee; ++ii) {
      CallInst* CI = dyn_cast<CallInst>(*ii);
      if (CI && CI->getCalledFunction() == F)
        if (hasConstArgs(CI))
          return true;
    }
    return false;
  }

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
  
  class IndCloner : public ModulePass {
  public:
    bool runOnModule(Module& M) {
      
      bool changed = false;

      //first figure out how functions are used
      std::set<Function*> DirectCalls;
      std::set<Function*> Unknowns;
      std::set<Function*> TakesPointers;
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
          if (isDirect(MI))
            DirectCalls.insert(MI);
          if (isUnknown(MI))
            Unknowns.insert(MI);
          if (hasPointer(MI))
            TakesPointers.insert(MI);
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
        }
      }
      return changed;
    }
  };

  class CSCloner : public ModulePass {
  public:
    bool runOnModule(Module& M) {
      
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
  };
  class DepthCloner : public ModulePass {
  public:
    bool runOnModule(Module& M) {
      
      bool changed = false;

      //first figure out how functions are used
      std::set<Function*> DirectCalls;
      std::set<Function*> Unknowns;
      std::set<Function*> TakesPointers;
      std::vector<std::string> IgnoreList;
      IgnoreList.push_back("kmalloc");
      IgnoreList.push_back("__vmalloc");
      IgnoreList.push_back("kmem_cache_alloc");
      IgnoreList.push_back("__alloc_bootmem");
      IgnoreList.push_back(" __get_free_pages");
      IgnoreList.push_back("kfree");
      IgnoreList.push_back("vfree");
      IgnoreList.push_back("free_pages");

      std::map<Function*, std::set<Function*> > Callers;
      typedef std::map<Function*, std::set<Function*> >::iterator citer;
      std::map<Function*, int> Level;

      for (Module::iterator MI = M.begin(), ME = M.end();
           MI != ME; ++MI)
        if (!MI->isExternal() &&
            std::find(IgnoreList.begin(), IgnoreList.end(), MI->getName()) == IgnoreList.end()) {
          if (isDirect(MI))
            DirectCalls.insert(MI);
          if (isUnknown(MI))
            Unknowns.insert(MI);
          if (hasPointer(MI))
            TakesPointers.insert(MI);
          for (Function::iterator FI = MI->begin(), FE = MI->end();
               FI != FE; ++FI)
            for (BasicBlock::iterator BI = FI->begin(), BE = FI->end();
                 BI != BE; ++BI) {
              if (isa<CallInst>(BI) && isa<Function>(BI->getOperand(0)) &&
                  BI->getOperand(0) != MI)
                Callers[MI].insert(cast<Function>(BI->getOperand(0)));
            }
        }

      //this doesn't handle mutual recursion
      bool c;
      do {
        c = false;
        for (citer ii = Callers.begin(), ee = Callers.end(); ii != ee; ++ii)
          for (std::set<Function*>::iterator i = ii->second.begin(), e = ii->second.end();
               i != e; ++i) {
            if (Level[ii->first] <= Level[*i] && Level[ii->first] <= CloneDepth) {
              ii->second.insert(Callers[*i].begin(), Callers[*i].end());
              c = true;
              Level[ii->first] = Level[*i] + 1;
            }
          }
      } while (c);
      
      //now think about replicating some functions
      for (Module::iterator MI = M.begin(), ME = M.end();
           MI != ME; ++MI) {
        if(TakesPointers.find(MI) != TakesPointers.end()) {

          //if it is a fixed depth, clone it
          if (Level[MI] <= CloneDepth && !MI->hasOneUse() && !MI->use_empty()) {
            for (Value::use_iterator ii = MI->use_begin(), ee = MI->use_end();
                 ii != ee; ++ii) {
              CallInst* CI = dyn_cast<CallInst>(*ii);
              if (CI && CI->getCalledFunction() == MI) {
                Function* FNew = clone(MI);
                ++DepthClone;
                changed = true;
                CI->setOperand(0, FNew);
              }
            }
          }
        }
      }
      return changed;
    }
  };

  RegisterPass<CSCloner>  X("csclone", "Cloning for Context Sensitivity");
  RegisterPass<IndCloner> Y("indclone", "Cloning for Context Sensitivity");
  RegisterPass<DepthCloner> Z("depthclone", "Cloning for Context Sensitivity");

}
