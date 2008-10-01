//===- BottomUpClosure.cpp - Compute bottom-up interprocedural closure ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the BUDataStructures class, which represents the
// Bottom-Up Interprocedural closure of the data structure graph over the
// program.  This is useful for applications like pool allocation, but **not**
// applications like alias analysis.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "bu_dsa"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Timer.h"
using namespace llvm;

namespace {
  STATISTIC (MaxSCC, "Maximum SCC Size in Call Graph");
  STATISTIC (NumBUInlines, "Number of graphs inlined");
  STATISTIC (NumCallEdges, "Number of 'actual' call edges");

  RegisterPass<BUDataStructures>
  X("dsa-bu", "Bottom-up Data Structure Analysis");
}

char BUDataStructures::ID;

// run - Calculate the bottom up data structure graphs for each function in the
// program.
//
bool BUDataStructures::runOnModule(Module &M) {
  StdLibDataStructures &LocalDSA = getAnalysis<StdLibDataStructures>();
  setGraphSource(&LocalDSA);
  setTargetData(LocalDSA.getTargetData());
  setGraphClone(false);
  GlobalECs = LocalDSA.getGlobalECs();

  GlobalsGraph = new DSGraph(LocalDSA.getGlobalsGraph(), GlobalECs);
  GlobalsGraph->setPrintAuxCalls();

  std::vector<Function*> Stack;
  hash_map<Function*, unsigned> ValMap;
  unsigned NextID = 1;

  Function *MainFunc = M.getFunction("main");
  if (MainFunc) {
    calculateGraphs(MainFunc, Stack, NextID, ValMap);
    CloneAuxIntoGlobal(getDSGraph(*MainFunc));
  }

  // Calculate the graphs for any functions that are unreachable from main...
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I)
    if (!I->isDeclaration() && !DSInfo.count(I)) {
      if (MainFunc)
        DOUT << "*** BU: Function unreachable from main: "
             << I->getName() << "\n";
      calculateGraphs(I, Stack, NextID, ValMap);     // Calculate all graphs.
      CloneAuxIntoGlobal(getDSGraph(*I));
    }

  //Be sure to get the all unresolved call sites
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I)
    if (!I->isDeclaration() && InlinedSomewhere.find(I) == InlinedSomewhere.end())
      CloneAuxIntoGlobal(getDSGraph(*I));
  InlinedSomewhere.clear();

  // If we computed any temporary indcallgraphs, free them now.
  for (std::map<std::vector<Function*>,
         std::pair<DSGraph*, std::vector<DSNodeHandle> > >::iterator I =
         IndCallGraphMap.begin(), E = IndCallGraphMap.end(); I != E; ++I) {
    I->second.second.clear();  // Drop arg refs into the graph.
    delete I->second.first;
  }
  IndCallGraphMap.clear();

  // At the end of the bottom-up pass, the globals graph becomes complete.
  // FIXME: This is not the right way to do this, but it is sorta better than
  // nothing!  In particular, externally visible globals and unresolvable call
  // nodes at the end of the BU phase should make things that they point to
  // incomplete in the globals graph.
  //

  finalizeGlobals();

  GlobalsGraph->removeTriviallyDeadNodes(true);
  GlobalsGraph->maskIncompleteMarkers();

  // Mark external globals incomplete.
  GlobalsGraph->markIncompleteNodes(DSGraph::IgnoreGlobals);

  formGlobalECs();

  // Merge the globals variables (not the calls) from the globals graph back
  // into the main function's graph so that the main function contains all of
  // the information about global pools and GV usage in the program.
  if (MainFunc && !MainFunc->isDeclaration()) {
    DSGraph &MainGraph = getOrCreateGraph(MainFunc);
    const DSGraph &GG = *MainGraph.getGlobalsGraph();
    ReachabilityCloner RC(MainGraph, GG,
                          DSGraph::DontCloneCallNodes |
                          DSGraph::DontCloneAuxCallNodes);

    // Clone the global nodes into this graph.
    for (DSScalarMap::global_iterator I = GG.getScalarMap().global_begin(),
           E = GG.getScalarMap().global_end(); I != E; ++I)
      if (isa<GlobalVariable>(*I))
        RC.getClonedNH(GG.getNodeForValue(*I));

    MainGraph.maskIncompleteMarkers();
    MainGraph.markIncompleteNodes(DSGraph::MarkFormalArgs |
                                  DSGraph::IgnoreGlobals);
  }

  NumCallEdges += ActualCallees.size();

  return false;
}

static inline bool nodeContainsExternalFunction(const DSNode *N) {
  std::vector<Function*> Funcs;
  N->addFullFunctionList(Funcs);
  for (unsigned i = 0, e = Funcs.size(); i != e; ++i)
    if (Funcs[i]->isDeclaration()) return true;
  return false;
}

void BUDataStructures::finalizeGlobals(void) {
  // Any unresolved call can be removed (resolved) if it does not contain
  // external functions and it is not reachable from any call that does
  // contain external functions
  std::set<DSCallSite> GoodCalls, BadCalls;
  for (DSGraph::afc_iterator ii = GlobalsGraph->afc_begin(), 
         ee = GlobalsGraph->afc_end(); ii != ee; ++ii)
    if (ii->isDirectCall() ||
        nodeContainsExternalFunction(ii->getCalleeNode()))
      BadCalls.insert(*ii);
    else
      GoodCalls.insert(*ii);
  hash_set<const DSNode*> reachable;
  for (std::set<DSCallSite>::iterator ii = BadCalls.begin(),
         ee = BadCalls.end(); ii != ee; ++ii) {
    ii->getRetVal().getNode()->markReachableNodes(reachable);
    for (unsigned x = 0; x < ii->getNumPtrArgs(); ++x)
      ii->getPtrArg(x).getNode()->markReachableNodes(reachable);
  }
  for (std::set<DSCallSite>::iterator ii = GoodCalls.begin(),
         ee = GoodCalls.end(); ii != ee; ++ii)
    if (reachable.find(ii->getCalleeNode()) == reachable.end())
      GlobalsGraph->getAuxFunctionCalls()
        .erase(std::find(GlobalsGraph->getAuxFunctionCalls().begin(),
                         GlobalsGraph->getAuxFunctionCalls().end(),
                         *ii));
  GlobalsGraph->getScalarMap().clear_scalars();
}

static void GetAllCallees(const DSCallSite &CS,
                          std::vector<Function*> &Callees) {
  if (CS.isDirectCall()) {
    if (!CS.getCalleeFunc()->isDeclaration())
      Callees.push_back(CS.getCalleeFunc());
  } else if (!CS.getCalleeNode()->isIncompleteNode()) {
    // Get all callees.
    unsigned OldSize = Callees.size();
    CS.getCalleeNode()->addFullFunctionList(Callees);
    
    // If any of the callees are unresolvable, remove the whole batch!
    for (unsigned i = OldSize; i != Callees.size(); )
      if (Callees[i]->isDeclaration()) {
        Callees.erase(Callees.begin()+i);
      } else 
        ++i;
  }
}

static void GetAnyCallees(const DSCallSite &CS,
                          std::vector<Function*> &Callees) {
  if (CS.isDirectCall()) {
    if (!CS.getCalleeFunc()->isDeclaration())
      Callees.push_back(CS.getCalleeFunc());
  } else {
    // Get all callees.
    unsigned OldSize = Callees.size();
    CS.getCalleeNode()->addFullFunctionList(Callees);
    
    // If any of the callees are unresolvable, remove the whole batch!
    for (unsigned i = OldSize, e = Callees.size(); i != e; ++i)
      if (Callees[i]->isDeclaration()) {
        Callees.erase(Callees.begin()+OldSize, Callees.end());
        return;
      }
  }
}

/// GetAllAuxCallees - Return a list containing all of the resolvable callees in
/// the aux list for the specified graph in the Callees vector.
static void GetAllAuxCallees(DSGraph &G, std::vector<Function*> &Callees) {
  Callees.clear();
  for (DSGraph::afc_iterator I = G.afc_begin(), E = G.afc_end(); I != E; ++I)
    GetAllCallees(*I, Callees);
}

/// GetAnyAuxCallees - Return a list containing all of the callees in
/// the aux list for the specified graph in the Callees vector.
static void GetAnyAuxCallees(DSGraph &G, std::vector<Function*> &Callees) {
  Callees.clear();
  for (DSGraph::afc_iterator I = G.afc_begin(), E = G.afc_end(); I != E; ++I)
    GetAnyCallees(*I, Callees);
}

unsigned BUDataStructures::calculateGraphs(Function *F,
                                           std::vector<Function*> &Stack,
                                           unsigned &NextID,
                                           hash_map<Function*, unsigned> &ValMap) {
  assert(!ValMap.count(F) && "Shouldn't revisit functions!");
  unsigned Min = NextID++, MyID = Min;
  ValMap[F] = Min;
  Stack.push_back(F);

  // FIXME!  This test should be generalized to be any function that we have
  // already processed, in the case when there isn't a main or there are
  // unreachable functions!
  if (F->isDeclaration()) {   // sprintf, fprintf, sscanf, etc...
    // No callees!
    Stack.pop_back();
    ValMap[F] = ~0;
    return Min;
  }

  DSGraph &Graph = getOrCreateGraph(F);

  // Find all callee functions.
  std::vector<Function*> CalleeFunctions;
  GetAnyAuxCallees(Graph, CalleeFunctions);

  // The edges out of the current node are the call site targets...
  for (unsigned i = 0, e = CalleeFunctions.size(); i != e; ++i) {
    Function *Callee = CalleeFunctions[i];
    unsigned M;
    // Have we visited the destination function yet?
    hash_map<Function*, unsigned>::iterator It = ValMap.find(Callee);
    if (It == ValMap.end())  // No, visit it now.
      M = calculateGraphs(Callee, Stack, NextID, ValMap);
    else                    // Yes, get it's number.
      M = It->second;
    if (M < Min) Min = M;
  }

  assert(ValMap[F] == MyID && "SCC construction assumption wrong!");
  if (Min != MyID)
    return Min;         // This is part of a larger SCC!

  // If this is a new SCC, process it now.
  if (Stack.back() == F) {           // Special case the single "SCC" case here.
    DOUT << "Visiting single node SCC #: " << MyID << " fn: "
         << F->getName() << "\n";
    Stack.pop_back();
    DOUT << "  [BU] Calculating graph for: " << F->getName()<< "\n";
    calculateGraph(Graph);
    DOUT << "  [BU] Done inlining: " << F->getName() << " ["
         << Graph.getGraphSize() << "+" << Graph.getAuxFunctionCalls().size()
         << "]\n";

    if (MaxSCC < 1) MaxSCC = 1;

    // Should we revisit the graph?  Only do it if there are now new resolvable
    // callees.
    std::vector<Function*> CalleeFunctionsNew;
    GetAnyAuxCallees(Graph, CalleeFunctionsNew);
    if (CalleeFunctionsNew.size() > CalleeFunctions.size()) {
      DOUT << "Recalculating " << F->getName() << " due to new knowledge\n";
      ValMap.erase(F);
      return calculateGraphs(F, Stack, NextID, ValMap);
    } else {
      ValMap[F] = ~0U;
    }
    return MyID;

  } else {
    // SCCFunctions - Keep track of the functions in the current SCC
    //
    std::vector<DSGraph*> SCCGraphs;

    unsigned SCCSize = 1;
    Function *NF = Stack.back();
    ValMap[NF] = ~0U;
    DSGraph &SCCGraph = getDSGraph(*NF);

    // First thing first, collapse all of the DSGraphs into a single graph for
    // the entire SCC.  Splice all of the graphs into one and discard all of the
    // old graphs.
    //
    while (NF != F) {
      Stack.pop_back();
      NF = Stack.back();
      ValMap[NF] = ~0U;

      DSGraph &NFG = getDSGraph(*NF);

      // Update the Function -> DSG map.
      for (DSGraph::retnodes_iterator I = NFG.retnodes_begin(),
             E = NFG.retnodes_end(); I != E; ++I)
        DSInfo[I->first] = &SCCGraph;

      SCCGraph.spliceFrom(NFG);
      delete &NFG;

      ++SCCSize;
    }
    Stack.pop_back();

    DOUT << "Calculating graph for SCC #: " << MyID << " of size: "
         << SCCSize << "\n";

    // Compute the Max SCC Size.
    if (MaxSCC < SCCSize)
      MaxSCC = SCCSize;

    // Clean up the graph before we start inlining a bunch again...
    SCCGraph.removeDeadNodes(DSGraph::KeepUnreachableGlobals);

    // Now that we have one big happy family, resolve all of the call sites in
    // the graph...
    calculateGraph(SCCGraph);
    DOUT << "  [BU] Done inlining SCC  [" << SCCGraph.getGraphSize()
         << "+" << SCCGraph.getAuxFunctionCalls().size() << "]\n"
         << "DONE with SCC #: " << MyID << "\n";

    // We never have to revisit "SCC" processed functions...
    return MyID;
  }

  return MyID;  // == Min
}

void BUDataStructures::CloneAuxIntoGlobal(DSGraph& G) {
  DSGraph& GG = *G.getGlobalsGraph();
  ReachabilityCloner RC(GG, G, 0);

  for(DSGraph::afc_iterator ii = G.afc_begin(), ee = G.afc_end();
      ii != ee; ++ii)
    GG.getAuxFunctionCalls().push_front(RC.cloneCallSite(*ii));
}

// releaseMemory - If the pass pipeline is done with this pass, we can release
// our memory... here...
//
void BUDataStructures::releaseMyMemory() {
  for (hash_map<Function*, DSGraph*>::iterator I = DSInfo.begin(),
         E = DSInfo.end(); I != E; ++I) {
    I->second->getReturnNodes().erase(I->first);
    if (I->second->getReturnNodes().empty())
      delete I->second;
  }

  // Empty map so next time memory is released, data structures are not
  // re-deleted.
  DSInfo.clear();
  delete GlobalsGraph;
  GlobalsGraph = 0;
}

void BUDataStructures::calculateGraph(DSGraph &Graph) {
  // If this graph contains the main function, clone the globals graph into this
  // graph before we inline callees and other fun stuff.
  bool ContainsMain = false;
  DSGraph::ReturnNodesTy &ReturnNodes = Graph.getReturnNodes();

  for (DSGraph::ReturnNodesTy::iterator I = ReturnNodes.begin(),
         E = ReturnNodes.end(); I != E; ++I)
    if (I->first->hasExternalLinkage() && I->first->getName() == "main") {
      ContainsMain = true;
      break;
    }

  // If this graph contains main, copy the contents of the globals graph over.
  // Note that this is *required* for correctness.  If a callee contains a use
  // of a global, we have to make sure to link up nodes due to global-argument
  // bindings.
  if (ContainsMain) {
    const DSGraph &GG = *Graph.getGlobalsGraph();
    ReachabilityCloner RC(Graph, GG,
                          DSGraph::DontCloneCallNodes |
                          DSGraph::DontCloneAuxCallNodes);

    // Clone the global nodes into this graph.
    for (DSScalarMap::global_iterator I = GG.getScalarMap().global_begin(),
           E = GG.getScalarMap().global_end(); I != E; ++I)
      if (isa<GlobalVariable>(*I))
        RC.getClonedNH(GG.getNodeForValue(*I));
  }


  // Move our call site list into TempFCs so that inline call sites go into the
  // new call site list and doesn't invalidate our iterators!
  std::list<DSCallSite> TempFCs;
  std::list<DSCallSite> &AuxCallsList = Graph.getAuxFunctionCalls();
  TempFCs.swap(AuxCallsList);

  bool Printed = false;
  std::vector<Function*> CalledFuncs;
  while (!TempFCs.empty()) {
    DSCallSite &CS = *TempFCs.begin();

    CalledFuncs.clear();

    // Fast path for noop calls.  Note that we don't care about merging globals
    // in the callee with nodes in the caller here.
    if (CS.getRetVal().isNull() && CS.getNumPtrArgs() == 0) {
      TempFCs.erase(TempFCs.begin());
      continue;
    }

    GetAllCallees(CS, CalledFuncs);
    bool isComplete = true;

    if (CalledFuncs.empty()) {
      // Remember that we could not resolve this yet!
      AuxCallsList.splice(AuxCallsList.end(), TempFCs, TempFCs.begin());
      isComplete = false;
      GetAnyCallees(CS, CalledFuncs);
    }
    
    if (CalledFuncs.empty())
      continue;

    DSGraph *GI;
    Instruction *TheCall = CS.getCallSite().getInstruction();
    
    if (CalledFuncs.size() == 1 && (isComplete || hasDSGraph(*CalledFuncs[0]))) {
      Function *Callee = CalledFuncs[0];
      ActualCallees.insert(std::make_pair(TheCall, Callee));
      if (isComplete) InlinedSomewhere.insert(Callee);

      // Get the data structure graph for the called function.
      GI = &getDSGraph(*Callee);  // Graph to inline
      DOUT << "    Inlining graph for " << Callee->getName()
           << "[" << GI->getGraphSize() << "+"
           << GI->getAuxFunctionCalls().size() << "] into '"
           << Graph.getFunctionNames() << "' [" << Graph.getGraphSize() <<"+"
           << Graph.getAuxFunctionCalls().size() << "]\n";
      Graph.mergeInGraph(CS, *Callee, *GI,
                         DSGraph::StripAllocaBit|DSGraph::DontCloneCallNodes|
                         (isComplete?0:DSGraph::DontCloneAuxCallNodes));
      ++NumBUInlines;
    } else {
      if (!Printed)
        DEBUG(std::cerr << "In Fns: " << Graph.getFunctionNames() << "\n");
      DEBUG(std::cerr << "  calls " << CalledFuncs.size()
            << " fns from site: " << CS.getCallSite().getInstruction()
            << "  " << *CS.getCallSite().getInstruction());
      DEBUG(std::cerr << "   Fns =");
      unsigned NumPrinted = 0;
      
      for (std::vector<Function*>::iterator I = CalledFuncs.begin(),
             E = CalledFuncs.end(); I != E; ++I) {
        if (NumPrinted++ < 8) DOUT << " " << (*I)->getName();
        
        // Add the call edges to the call graph.
        ActualCallees.insert(std::make_pair(TheCall, *I));
      }
      DOUT << "\n";
      
      if (!isComplete) {
        for (unsigned x = 0; x < CalledFuncs.size(); )
          if (!hasDSGraph(*CalledFuncs[x]))
            CalledFuncs.erase(CalledFuncs.begin() + x);
          else
            ++x;
        if (!CalledFuncs.size())
          continue;
      }

      // See if we already computed a graph for this set of callees.
      std::sort(CalledFuncs.begin(), CalledFuncs.end());
      std::pair<DSGraph*, std::vector<DSNodeHandle> > &IndCallGraph =
        IndCallGraphMap[CalledFuncs];
      
      if (IndCallGraph.first == 0) {
        std::vector<Function*>::iterator I = CalledFuncs.begin(),
          E = CalledFuncs.end();
        
        // Start with a copy of the first graph.
        GI = IndCallGraph.first = new DSGraph(getDSGraph(**I), GlobalECs);
        GI->setGlobalsGraph(Graph.getGlobalsGraph());
        std::vector<DSNodeHandle> &Args = IndCallGraph.second;
        
        // Get the argument nodes for the first callee.  The return value is
        // the 0th index in the vector.
        GI->getFunctionArgumentsForCall(*I, Args);
        
        // Merge all of the other callees into this graph.
        for (++I; I != E; ++I) {
          if (isComplete) InlinedSomewhere.insert(*I);
          // If the graph already contains the nodes for the function, don't
          // bother merging it in again.
          if (!GI->containsFunction(*I)) {
            GI->cloneInto(getDSGraph(**I));
            ++NumBUInlines;
          }
          
          std::vector<DSNodeHandle> NextArgs;
          GI->getFunctionArgumentsForCall(*I, NextArgs);
          unsigned i = 0, e = Args.size();
          for (; i != e; ++i) {
            if (i == NextArgs.size()) break;
            Args[i].mergeWith(NextArgs[i]);
          }
          for (e = NextArgs.size(); i != e; ++i)
            Args.push_back(NextArgs[i]);
        }
        
        // Clean up the final graph!
        GI->removeDeadNodes(DSGraph::KeepUnreachableGlobals);
      } else {
        DOUT << "***\n*** RECYCLED GRAPH ***\n***\n";
      }
      
      GI = IndCallGraph.first;
      
      // Merge the unified graph into this graph now.
      DOUT << "    Inlining multi callee graph "
           << "[" << GI->getGraphSize() << "+"
           << GI->getAuxFunctionCalls().size() << "] into '"
           << Graph.getFunctionNames() << "' [" << Graph.getGraphSize() <<"+"
           << Graph.getAuxFunctionCalls().size() << "]\n";
      
      Graph.mergeInGraph(CS, IndCallGraph.second, *GI,
                         DSGraph::StripAllocaBit |
                         DSGraph::DontCloneCallNodes|
                         (isComplete?0:DSGraph::DontCloneAuxCallNodes));
      ++NumBUInlines;
    }
    if (isComplete)
      TempFCs.erase(TempFCs.begin());
  }

  // Recompute the Incomplete markers
  Graph.maskIncompleteMarkers();
  Graph.markIncompleteNodes(DSGraph::MarkFormalArgs);

  // Delete dead nodes.  Treat globals that are unreachable but that can
  // reach live nodes as live.
  Graph.removeDeadNodes(DSGraph::KeepUnreachableGlobals);

  // When this graph is finalized, clone the globals in the graph into the
  // globals graph to make sure it has everything, from all graphs.
  DSScalarMap &MainSM = Graph.getScalarMap();
  ReachabilityCloner RC(*GlobalsGraph, Graph, DSGraph::StripAllocaBit);

  // Clone everything reachable from globals in the function graph into the
  // globals graph.
  for (DSScalarMap::global_iterator I = MainSM.global_begin(),
         E = MainSM.global_end(); I != E; ++I)
    RC.getClonedNH(MainSM[*I]);

  //Graph.writeGraphToFile(cerr, "bu_" + F.getName());
}

static const Function *getFnForValue(const Value *V) {
  if (const Instruction *I = dyn_cast<Instruction>(V))
    return I->getParent()->getParent();
  else if (const Argument *A = dyn_cast<Argument>(V))
    return A->getParent();
  else if (const BasicBlock *BB = dyn_cast<BasicBlock>(V))
    return BB->getParent();
  return 0;
}

/// deleteValue/copyValue - Interfaces to update the DSGraphs in the program.
/// These correspond to the interfaces defined in the AliasAnalysis class.
void BUDataStructures::deleteValue(Value *V) {
  if (const Function *F = getFnForValue(V)) {  // Function local value?
    // If this is a function local value, just delete it from the scalar map!
    getDSGraph(*F).getScalarMap().eraseIfExists(V);
    return;
  }

  if (Function *F = dyn_cast<Function>(V)) {
    assert(getDSGraph(*F).getReturnNodes().size() == 1 &&
           "cannot handle scc's");
    delete DSInfo[F];
    DSInfo.erase(F);
    return;
  }

  assert(!isa<GlobalVariable>(V) && "Do not know how to delete GV's yet!");
}

void BUDataStructures::copyValue(Value *From, Value *To) {
  if (From == To) return;
  if (const Function *F = getFnForValue(From)) {  // Function local value?
    // If this is a function local value, just delete it from the scalar map!
    getDSGraph(*F).getScalarMap().copyScalarIfExists(From, To);
    return;
  }

  if (Function *FromF = dyn_cast<Function>(From)) {
    Function *ToF = cast<Function>(To);
    assert(!DSInfo.count(ToF) && "New Function already exists!");
    DSGraph *NG = new DSGraph(getDSGraph(*FromF), GlobalECs);
    DSInfo[ToF] = NG;
    assert(NG->getReturnNodes().size() == 1 && "Cannot copy SCC's yet!");

    // Change the Function* is the returnnodes map to the ToF.
    DSNodeHandle Ret = NG->retnodes_begin()->second;
    NG->getReturnNodes().clear();
    NG->getReturnNodes()[ToF] = Ret;
    return;
  }

  if (const Function *F = getFnForValue(To)) {
    DSGraph &G = getDSGraph(*F);
    G.getScalarMap().copyScalarIfExists(From, To);
    return;
  }

  cerr << *From;
  cerr << *To;
  assert(0 && "Do not know how to copy this yet!");
  abort();
}
