//===- PoolRegisterElimination.cpp ---------------------------------------- --//
// 
//                          The SAFECode Compiler 
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
//  This pass eliminates unnessary poolregister() / poolunregister() in the
//  code. Redundant poolregister() happens when there are no boundscheck() /
//  poolcheck() on a certain GEP, possibly all of these checks are lowered to
//  exact checks.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "poolreg-elim"
#include "safecode/OptimizeChecks.h"
#include "safecode/Support/AllocatorInfo.h"
#include "SCUtils.h"

#include "dsa/DSSupport.h"

#include "llvm/ADT/Statistic.h"


NAMESPACE_SC_BEGIN

static RegisterPass<PoolRegisterElimination>
X ("poolreg-elim", "Pool Register Eliminiation");

// Pass Statistics
namespace {
  STATISTIC (RemovedRegistration,
  "Number of object registrations/deregistrations removed");
}

//
// Method: findCheckedAliasSets()
//
// Description:
//  This method finds all alias sets which contain pointers that have been used
//  in run-time checks that require a splay-tree lookup.
//
void
PoolRegisterElimination::findCheckedAliasSets () {
  // FIXME: The list of intrinsics should be selected via scanning through the
  // intrinsic lists with specified flags.
  const char * splayTreeCheckIntrinsics[] = {
    "sc.lscheck",
    "sc.lscheckui",
    "sc.lscheckalign",
    "sc.lscheckalignui",
    "sc.boundscheck",
    "sc.boundscheckui"
  };

  //
  // Find all of the pointers that are used by run-time checks which require an
  // object lookup.  Mark their alias sets as being checked; this ensures that
  // any pointers aliasing with checked pointers are registered.
  //
  for (size_t i = 0;
       i < sizeof(splayTreeCheckIntrinsics) / sizeof (const char*);
       ++i) {
    markUsedAliasSet(splayTreeCheckIntrinsics[i], usedSet);
  }

  return;
}

//
// Method: findSafeGlobals()
//
// Description:
//  Find global variables that do not escape into memory or external code.
//
template<typename insert_iterator>
void
PoolRegisterElimination::findSafeGlobals (Module & M, insert_iterator InsertPt) {
  for (Module::global_iterator GV = M.global_begin();
       GV != M.global_end();
       ++GV) {
    if (!escapesToMemory (GV))
      InsertPt = GV;
  }

  return;
}

bool
PoolRegisterElimination::runOnModule(Module & M) {
  //
  // Get the set of safe globals.
  //
  findSafeGlobals (M, std::inserter (SafeGlobals, SafeGlobals.begin()));

  //
  // Get access to prequisite analysis passes.
  //
  intrinsic = &getAnalysis<InsertSCIntrinsic>();

  //
  // List of registration intrinsics.
  //
  // FIXME:
  //  It is possible that removeUnusedRegistration() will properly detect
  //  that pointers *within* argv are not used.  This should be investigated
  //  before sc.pool_argvregister() is added back into the list.
  //
  // Note that sc.pool_argvregister() is not in this list.  This is because
  // it registers both the argv array and all the command line arguments whose
  // pointers are within the argv array.
  //
  const char * registerIntrinsics[] = {
    "sc.pool_register_global"
  };

  //
  // Remove all unused registrations.
  //
  unsigned numberOfIntrinsics=sizeof(registerIntrinsics) / sizeof (const char*);
  for (size_t i = 0; i < numberOfIntrinsics; ++i) {
    removeUnusedRegistrations (registerIntrinsics[i]);
  }

  //
  // Deallocate memory and return;
  //
  delete AST;
  SafeGlobals.clear();
  return true;
}

//
// Method: findFreedAliasSets()
//
// Description:
//  This method will find and record all the alias sets that have pointers that
//  have been used in deallocation functions.
//
void
DebugPoolRegisterElimination::findFreedAliasSets (void) {
  return;
}

bool
DebugPoolRegisterElimination::runOnModule(Module & M) {
  //
  // Clear out the set of used alias groups.
  //
  usedSet.clear();

  //
  // Get access to prequisite analysis passes.
  //
  intrinsic = &getAnalysis<InsertSCIntrinsic>();
  AA = &getAnalysis<AliasAnalysis>();
  AST = new AliasSetTracker(*AA);

  //
  // Find all alias sets that have a pointer that is passed to a run-time
  // check that does a splay-tree lookup.
  //
  findCheckedAliasSets();

  //
  // Find all alias sets that have a pointer that is freed.  Such pointers are
  // considered "used" since we need to do invalid free checks on them.
  //
  markUsedAliasSet ("sc.pool_unregister", usedSet);

  //
  // List of registration intrinsics.
  //
  // FIXME:
  //  It is possible that removeUnusedRegistration() will properly detect
  //  that pointers *within* argv are not used.  This should be investigated
  //  before sc.pool_argvregister() is added back into the list.
  //
  // Note that sc.pool_argvregister() is not in this list.  This is because
  // it registers both the argv array and all the command line arguments whose
  // pointers are within the argv array.
  //
  const char * registerIntrinsics[] = {
    "sc.pool_register_global",
    "sc.pool_register_stack",
    "sc.pool_unregister_stack",
  };

  //
  // Remove all unused registrations.
  //
  unsigned numberOfIntrinsics=sizeof(registerIntrinsics) / sizeof (const char*);
  for (size_t i = 0; i < numberOfIntrinsics; ++i) {
    removeUnusedRegistrations (registerIntrinsics[i]);
  }

  //
  // Deallocate memory and return;
  //
  delete AST;
  return true;
}

//
// Method: markUsedAliasSet
//
// Description:
//  This method takes the name of a SAFECode run-time function and determines
//  which alias sets are ever passed into the function.
//
// Inputs:
//  name - The name of the run-time function for which to find uses.
//
// Outputs:
//  set  - The set into which alias sets that use the function should go.
//
void
PoolRegisterElimination::markUsedAliasSet (const char * name,
                                           DenseSet<AliasSet*> & set) {
  Function * F = intrinsic->getIntrinsic(name).F;

  for(Value::use_iterator UI=F->use_begin(), UE=F->use_end(); UI != UE; ++UI) {
    CallInst * CI = cast<CallInst>(*UI);
    Value * checkedPtr = intrinsic->getValuePointer(CI);
    AliasSet & aliasSet = AST->getAliasSetForPointer(checkedPtr, 0);
    set.insert(&aliasSet);
  }
}

//
// Method: isSafeToRemove()
//
// Description:
//  Determine whether the registration for the specified pointer value can be
//  safely removed.
//
// Inputs:
//  Ptr - The pointer value that is registered.
//
// Return value:
//  true  - The registration of this value can be safely removed.
//  false - The registration of this value may not be safely removed.
//
bool
PoolRegisterElimination::isSafeToRemove (Value * Ptr) {
  //
  // We can remove registrations on global variables that don't escape to
  // memory.
  //
  if (GlobalVariable * GV = dyn_cast<GlobalVariable>(Ptr)) {
    if (SafeGlobals.count (GV))
      return true;
  }

  return false;
}

//
// Method: removeUnusedRegistration()
//
// Description:
//  This method take the name of a registration function and removes all
//  registrations made with that function for pointers that are never checked.
//
// Inputs:
//  name - The name of the registration intrinsic.
//
void
PoolRegisterElimination::removeUnusedRegistrations (const char * name) {
  //
  // Scan through all uses of each registration function and see if it can be
  // safely removed.  If so, schedule it for removal.
  //
  std::vector<CallInst*> toBeRemoved;
  Function * F = intrinsic->getIntrinsic(name).F;

  //
  // Look for and record all registrations that can be deleted.
  //
  for (Value::use_iterator UI=F->use_begin(), UE=F->use_end();
       UI != UE;
       ++UI) {
    CallInst * CI = cast<CallInst>(*UI);
    if (isSafeToRemove (intrinsic->getValuePointer(CI))) {
      toBeRemoved.push_back(CI);
    }
  }

  //
  // Update the statistics.
  //
  RemovedRegistration += toBeRemoved.size();

  //
  // Remove the unnecesary registrations.
  //
  std::vector<CallInst*>::iterator it, end;
  for (it = toBeRemoved.begin(), end = toBeRemoved.end(); it != end; ++it) {
    (*it)->eraseFromParent();
  }
}

char PoolRegisterElimination::ID      = 0;
char DebugPoolRegisterElimination::ID = 0;

NAMESPACE_SC_END