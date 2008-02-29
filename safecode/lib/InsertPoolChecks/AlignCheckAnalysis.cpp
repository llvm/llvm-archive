//===-- AlignCheckAnalysis.cpp - Find DSNodes needing alignment checks ----===//
//
//                     SAFECode
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a simple analysis pass that determines which DSNodes
// need alignment checks.
//
//===----------------------------------------------------------------------===//

#include "safecode/Config/config.h"
#include "InsertPoolChecks.h"
#include "SCUtils.h"
#include "llvm/Instruction.h"
#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/VectorExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ConstantRange.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/LoopInfo.h" 
#include <iostream>
#include <set>

using namespace llvm;

namespace {
  RegisterPass<AlignCheckAnalysis> aca ("aca", "Alignment Check Analysis");
}

////////////////////////////////////////////////////////////////////////////
// Class: AlignCheckAnalysis
////////////////////////////////////////////////////////////////////////////

//
// Method: runOnModule()
//
// Description:
//  This method is called by the pass manager.
//
bool
AlignCheckAnalysis::runOnModule (Module & M) {
  // Retrieve the analysis results from other passes
  TDPass    = &getAnalysis<TDDataStructures>();

  //
  // Scan every DSGraph in the module to look for DSNodes requiring alignment.
  //
  Module::iterator mI = M.begin(), mE = M.end();
  for ( ; mI != mE; ++mI) {
    Function *F = mI;

    // Skip functions that are external
    if (F->isExternal()) continue;

    // Skip the poolcheckglobals() function because it won't have a DSGraph
    if (F->getName() == "poolcheckglobals") continue;

    // Create a MetaPool variable for each DSNode in the DSGraph.
    DSGraph & TDG = getDSGraph(*F);
    DSGraph::node_iterator NI = TDG.node_begin(), NE = TDG.node_end();
    while (NI != NE) {
      addLinksNeedingAlignment (NI);
      ++NI;
    }
  }

  return false;
}

//
// Method: getDSGraph()
//
// Description:
//  Return the DSGraph for the given function.  This method automatically
//  selects the correct pass to query for the graph based upon whether we're
//  doing user-space or kernel analysis.
//
DSGraph &
AlignCheckAnalysis::getDSGraph(Function & F) {
#ifndef LLVA_KERNEL
  return equivPass->getDSGraph(F);
#else  
  return TDPass->getDSGraph(F);
#endif  
}

//
// Method: addLinksNeedingAlignment()
//
// Description:
//  Determine if this DSNode has any pointers to DSNodes which will require
//  alignment checks.  If so, add those DSNodes to the set of DSNodes needing
//  alignment checks.  Note that we do not determine if the *given* node needs
//  alignment checks.
//
void
AlignCheckAnalysis::addLinksNeedingAlignment (DSNode * Node) {
  //
  // Determine whether an alignment check is needed.  This occurs when a DSNode
  // is type unknown (collapsed) but has pointers to type known (uncollapsed)
  // DSNodes.
  //
  if ((Node) && (Node->isNodeCompletelyFolded())) {
    for (unsigned i = 0 ; i < Node->getNumLinks(); ++i) {
      DSNode * LinkNode = Node->getLink(i).getNode();
      if (LinkNode && (!(LinkNode->isNodeCompletelyFolded()))) {
        AlignmentNodes.insert (LinkNode);
      }
    }
  }
}

