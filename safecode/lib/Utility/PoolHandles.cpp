//===- PoolHandles.cpp - Passes for finding pointer attributes for SAFECode --//
// 
//                          The SAFECode Compiler 
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements several passes which ease the use of the automatic pool
// allocation transform.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "poolhandle"

#include "safecode/Config/config.h"
#include "safecode/PoolHandles.h"

NAMESPACE_SC_BEGIN

// Pass ID variables
char PoolMDPass::ID       = 0;
char RemovePoolMDPass::ID = 0;

//
// Method: createPoolMetaData()
//
// Description:
//  This method locates the pool for the specified value and creates a metadata
//  node that links the value with its pool.
//
// Inputs:
//  V - The value for which a pool metadata node should be created.
//  F - The function in which the pool should be found.
//
// Side effects:
//  This method will add the metata node to a container that is global to class
//  member methods.
//
void
PoolMDPass::createPoolMetaData (Value * V, Function * F) {
  //
  // Get the pool associated with the value.
  //
  PA::FuncInfo *FI = dsnPass->paPass->getFuncInfoOrClone(*F);
  Value * PH = dsnPass->getPoolHandle (V, F, *FI);
  assert (PH && "No pool handle for the specified value!\n");

  //
  // Get the DSNode information associated with the value.
  //
  DSNode* Node = dsnPass->getDSNode(V, F);
  assert (Node && "Value has no DSNode!\n");

  //
  // Create LLVM values representing the pointer's type, DSNode Flags, etc.
  //
  const Type * Int1Type  = Type::getInt1Ty (F->getParent()->getContext());
  const Type * Int32Type = Type::getInt32Ty(F->getParent()->getContext());
  Value * IsFolded = ConstantInt::get(Int1Type, Node->isNodeCompletelyFolded());
  Value * DSFlags = ConstantInt::get(Int32Type, Node->getNodeFlags());

  // 
  // Create a new metadata node that contains the pool handle and the value.
  // 
  Value * PoolMap[4] = {V, PH, IsFolded, DSFlags};
  MDNode * MD = MDNode::get (F->getParent()->getContext(), PoolMap, 4);

  //
  // Add the value to pool mapping to the set of mappings we've created so far.
  //
  ValueToPoolNodes.push_back (MD);
  return;
}

void
PoolMDPass::visitLoadInst (LoadInst & LI) {
  //
  // Create meta-data linking the dereferenced pointer with its pool.
  //
  Function * F = LI.getParent()->getParent();
  createPoolMetaData (LI.getPointerOperand(), F);
  return;
}

void
PoolMDPass::visitStoreInst (StoreInst & SI) {
  //
  // Create meta-data linking the dereferenced pointer with its pool.
  //
  Function * F = SI.getParent()->getParent();
  createPoolMetaData (SI.getPointerOperand(), F);
  return;
}

//
// Method: runOnModule()
//
// Description:
//  The LLVM pass manager will call this method when this pass is to be run on
//  a Module.
//
// Return value:
//  true  - The module was modified.
//  false - The module was not modified.
//
bool
PoolMDPass::runOnModule (Module &M) {
  //
  // Get a handle to the pool allocation pass.
  //
  dsnPass = &getAnalysis<DSNodePass>();

  //
  // Visit all instructions within the module to find all of the pool handles
  // we need.
  //
  visit (M);

  //
  // Create a global meta-data node that links to all the other meta-data.
  //
  Twine name ("SCValueMap");
  NamedMDNode * MD = NamedMDNode::Create (M.getContext(), name, 0, 0, &M);
  for (unsigned index = 0; index < ValueToPoolNodes.size(); ++index)
    MD->addElement (ValueToPoolNodes[index]);

  // Assume that we modified something
  return true;
}

bool
QueryPoolPass::runOnModule (Module & M) {
 //
  // Get the basic block metadata.  If there isn't any metadata, then no basic
  // block has been numbered.
  //
  const NamedMDNode * MD = M.getNamedMetadata ("SCValueMap");
  if (!MD) return false;

  //
  // Scan through all of the metadata and add the information in it into our
  // internal data structures.
  //
  for (unsigned index = 0; index < MD->getNumElements(); ++index) {
    //
    // Get one entry of meta-data.
    //
    MDNode * Node = dyn_cast<MDNode>(MD->getElement (index));
    assert (Node && "Wrong type of meta data!\n");

    //
    // Extract the information about this value from the metadata.
    //
    Value * V              = dyn_cast<Value>(Node->getElement (0));
    Value * PH             = dyn_cast<Value>(Node->getElement (1));
    ConstantInt * IsFolded = dyn_cast<ConstantInt>(Node->getElement (2));
    ConstantInt * DSFlags  = dyn_cast<ConstantInt>(Node->getElement (3));

    //
    // Do some assertions to make sure that everything is sane.
    //
    assert (V  && "MDNode first element is not a Value!\n");
    assert (PH && "MDNode second element is not a Pool Handle!\n");
    assert (IsFolded && "MDNode third element is not a constant integer!\n");
    assert (DSFlags && "MDNode fourth element is not a constant integer!\n");

    //
    // Add the values into the maps.
    //
    PoolMap[V]   = PH;
    FoldedMap[V] = !(IsFolded->isZero());
    FlagMap[V]   = DSFlags->getZExtValue();
  }

  return false;
}

//
// Method: getPool()
//
// Description:
//  Given an LLVM value, attempt to find the pool associated with that value.
//
Value *
QueryPoolPass::getPool (Value * V) {
  return PoolMap[V];
}

//
// Method: runOnModule()
//
// Description:
//  This is the entry point for our pass.  It removes the metadata created by
//  PoolMDPass.
//
// Return value:
//  true  - Metadata was removed from the Module.
//  false - No modifications were made to the Module.
//
bool
RemovePoolMDPass::runOnModule (Module & M) {
  //
  // Get the pool metadata.  If there isn't any metadata, then nothing needs to
  // be done.
  //  
  NamedMDNode * MD = M.getNamedMetadata ("SCValueMap");
  if (!MD) return false;
  
  //
  // Remove the metadata.
  //
  MD->eraseFromParent();
      
  //    
  // Assume we always modify the module.
  //
  return true;
} 

NAMESPACE_SC_END

