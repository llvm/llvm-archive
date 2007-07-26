//===- Devirt.cpp - Devirtualize using the sig match intrinsic in llva ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/IPO.h"
#include "dsa/CallTargets.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/ADT/Statistic.h"

#include <iostream>
#include <map>
#include <algorithm>
#include <iterator>

using namespace llvm;

namespace {

  static cl::opt<int>
  VirtualLimit("devirt-limit", cl::Hidden, cl::init(16),
               cl::desc("Maximum number of callees to devirtualize at a call site"));
  Statistic<> FuncAdded("devirt", "Number of bounce functions added");
  Statistic<> CSConvert("devirt", "Number of call sites converted");


  class Devirtualize : public ModulePass {


    std::map<std::pair<const Type*, std::vector<Function*> >, Function*> cache;
    int fnum;

    Function* buildBounce(CallSite cs, std::vector<Function*>& Targets, Module& M) {
      Value* ptr = cs.getCalledValue();
      const FunctionType* OrigType = 
        cast<FunctionType>(cast<PointerType>(ptr->getType())->getElementType());;
      ++FuncAdded;

      std::vector< const Type *> TP(OrigType->param_begin(), OrigType->param_end());
      TP.insert(TP.begin(), ptr->getType());
      const FunctionType* NewTy = FunctionType::get(OrigType->getReturnType(), TP, false);
      Function* F = new Function(NewTy, GlobalValue::InternalLinkage, "devirtbounce", &M);
      std::map<Function*, BasicBlock*> targets;

      F->arg_begin()->setName("funcPtr");
      std::vector<Value*> fargs;
      for(Function::arg_iterator ai = F->arg_begin(), ae = F->arg_end(); ai != ae; ++ai)
        if (ai != F->arg_begin()) {
          fargs.push_back(ai);
          ai->setName("arg");
        }

      for (std::vector<Function*>::iterator i = Targets.begin(), e = Targets.end();
           i != e; ++i) {
        Function* FL = *i;
        BasicBlock* BL = new BasicBlock(FL->getName(), F);
        targets[FL] = BL;

        //Make call
        Value* call = new CallInst(FL, fargs, "", BL);

        //return correctly
        if (OrigType->getReturnType() == Type::VoidTy)
          new ReturnInst(0, BL);
        else
          new ReturnInst(call, BL);
      }

      //hookup the test chain
      BasicBlock* tail = new BasicBlock("fail", F, &F->getEntryBlock());
      new CallInst(M.getOrInsertFunction("pchk_ind_fail", Type::VoidTy, NULL),
                   "", tail);
      new UnreachableInst(tail);
      
      for (std::vector<Function*>::iterator i = Targets.begin(), e = Targets.end();
           i != e; ++i) {
        BasicBlock* TB = targets[*i];
        BasicBlock* newB = new BasicBlock("test." + (*i)->getName(), F, &F->getEntryBlock());
        Value* p = F->arg_begin();
        SetCondInst* setcc = new SetCondInst(Instruction::SetEQ, *i, p, "sc", newB);
        new BranchInst(TB, tail, setcc, newB);
        tail = newB;
      }
      return F;
    }

  public:
    virtual bool runOnModule(Module &M) {
      CallTargetFinder* CTF = &getAnalysis<CallTargetFinder>();
      bool changed = false;

      Function* ams = M.getNamedFunction("llva_assert_match_sig");
      
      std::set<Value*> safecalls;

      for (Value::use_iterator ii = ams->use_begin(), ee = ams->use_end();
           ii != ee; ++ii) {
        if (CallInst* CI = dyn_cast<CallInst>(*ii)) {
          std::cerr << "Found safe call site in " 
                    << CI->getParent()->getParent()->getName() << "\n";
          Value* V = CI->getOperand(1);
          CI->eraseFromParent();
          do {
            safecalls.insert(V);
            if (CastInst* CV = dyn_cast<CastInst>(V))
              V = CV->getOperand(0);
            else V = 0;
          } while (V);
        }
      }

      std::vector<Instruction*> toDelete;

      for(std::set<Value*>::iterator i = safecalls.begin(), e = safecalls.end();
          i != e; ++i) {
        for (Value::use_iterator ii = (*i)->use_begin(), ie = (*i)->use_end();
             ii != ie; ++ii) {
          CallSite cs = CallSite::get(*ii);
          bool isSafeCall = cs.getInstruction() && 
            safecalls.find(cs.getCalledValue()) != safecalls.end();
          if (cs.getInstruction() && !cs.getCalledFunction() &&
              (isSafeCall || CTF->isComplete(cs))) {
            std::vector<Function*> Targets;
            for (std::vector<Function*>::iterator ii = CTF->begin(cs), ee = CTF->end(cs);
                 ii != ee; ++ii)
              if (!isSafeCall || (*ii)->getType() == cs.getCalledValue()->getType())
                Targets.push_back(*ii);
            
            if (Targets.size() > 0) {
              std::cerr << "Target count: " << Targets.size() << "\n";
              Function* NF = buildBounce(cs, Targets, M);
              changed = true;
              if (CallInst* ci = dyn_cast<CallInst>(cs.getInstruction())) {
                ++CSConvert;
                std::vector<Value*> Par(ci->op_begin(), ci->op_end());
                CallInst* cn = new CallInst(NF, Par,
                                            ci->getName() + ".dv", ci);
                ci->replaceAllUsesWith(cn);
                toDelete.push_back(ci);
              } else if (InvokeInst* ci = dyn_cast<InvokeInst>(cs.getInstruction())) {
                ++CSConvert;
                std::vector<Value*> Par(ci->op_begin(), ci->op_end());
                InvokeInst* cn = new InvokeInst(NF, ci->getNormalDest(),
                                                ci->getUnwindDest(),
                                                Par, ci->getName()+".dv",
                                                ci);
                ci->replaceAllUsesWith(cn);
                toDelete.push_back(ci);
              }
            }
          }
        }
      }
      for (std::vector<Instruction*>::iterator ii = toDelete.begin(), ee = toDelete.end();
           ii != ee; ++ii)
        (*ii)->eraseFromParent();
      return changed;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<CallTargetFinder>();
    }

  };

  RegisterPass<Devirtualize> X("devirt", "Devirtualization");

}
