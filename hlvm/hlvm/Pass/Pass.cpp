//===-- Pass Interface Class ------------------------------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/Pass/Pass.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::Pass::Pass.
//===----------------------------------------------------------------------===//

#include <hlvm/Pass/Pass.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/Base/Assert.h>

using namespace hlvm;
using namespace llvm;

namespace {

class PassManagerImpl : public PassManager
{
public:
  PassManagerImpl() : PassManager(), pre(), post() {}
  void addPass(Pass* p);
  virtual void runOn(AST* tree);
  virtual void runOnNode(Node* startAt);

  inline void runIfInterested(Pass* p, Node* n, Pass::TraversalKinds m);
  inline void runPreOrder(Node* n);
  inline void runPostOrder(Node* n);

  template<class NodeClass>
  inline void runOn(NodeClass* b);

private:
  std::vector<Pass*> pre;
  std::vector<Pass*> post;
  std::vector<Pass*> all;
};

void
PassManagerImpl::addPass(Pass* p)
{
  all.push_back(p);
  if (p->mode() & Pass::PreOrderTraversal)
    pre.push_back(p);
  if (p->mode() & Pass::PostOrderTraversal)
    post.push_back(p);
}

inline void
PassManagerImpl::runIfInterested(Pass* p, Node* n, Pass::TraversalKinds m)
{
  int interest = p->interest();
  if (interest == 0 ||
     ((interest & Pass::Type_Interest) && n->isType()) ||
     ((interest & Pass::Function_Interest) && n->isFunction()) ||
     ((interest & Pass::Block_Interest) && n->is(BlockID)) ||
     ((interest & Pass::Operator_Interest) && n->isOperator()) ||
     ((interest & Pass::Program_Interest) && n->is(ProgramID)) ||
     ((interest & Pass::Variable_Interest) && n->is(VariableID))
     ) {
    p->handle(n,m);
  }
}

inline void 
PassManagerImpl::runPreOrder(Node* n)
{
  std::vector<Pass*>::iterator I = pre.begin(), E = pre.end();
  while (I != E) {
    runIfInterested(*I,n,Pass::PreOrderTraversal);
    ++I;
  }
}

inline void 
PassManagerImpl::runPostOrder(Node* n)
{
  std::vector<Pass*>::iterator I = post.begin(), E = post.end();
  while (I != E) {
    runIfInterested(*I,n,Pass::PostOrderTraversal);
    ++I;
  }
}

template<> inline void
PassManagerImpl::runOn(ConstantValue* cst)
{
  hlvmAssert(cst && "Null constant?");
  hlvmAssert(isa<Constant>(cst));
  runPreOrder(cst);
  // FIXME: Eventually we'll have structured constants which need to have 
  // their contents examined as well.
  runPostOrder(cst);
}

template<> inline void
PassManagerImpl::runOn(Operator* op)
{
  hlvmAssert(op && "Null operator?");
  runPreOrder(op);
  size_t limit = op->getNumOperands();
  for (size_t i = 0; i < limit; ++i)
    runOn(op->getOperand(i));
  runPostOrder(op);
}

template<> inline void
PassManagerImpl::runOn(Block* b)
{
  hlvmAssert(b && "Null block?");
  runPreOrder(b);
  for (Block::iterator I = b->begin(), E = b->end(); I != E; ++I)
    runOn(cast<Operator>(*I)); // recurse, possibly!
  runPostOrder(b);
}

template<> inline void
PassManagerImpl::runOn(Linkable* l)
{
  runPreOrder(l);
  if (Function* F = dyn_cast<Function>(l))
    if (Block* B = F->getBlock())
      runOn(B);
  runPostOrder(l);
}

template<> inline void 
PassManagerImpl::runOn(Bundle* b)
{
  hlvmAssert(b && "Null bundle?");
  runPreOrder(b);
  for (Bundle::tlist_iterator TI = b->tlist_begin(), TE = b->tlist_end(); 
       TI != TE; ++TI) {
    runPreOrder(const_cast<Type*>(*TI));
    runPostOrder(const_cast<Type*>(*TI));
  }
  for (Bundle::clist_iterator CI = b->clist_begin(), CE = b->clist_end(); 
       CI != CE; ++CI) {
    runPreOrder(const_cast<Constant*>(*CI));
    if (Function* F = dyn_cast<Function>(*CI))
      if (Block* B = F->getBlock())
        runOn(B);
    runPostOrder(const_cast<Constant*>(*CI));
  }
  runPostOrder(b);
}

void PassManagerImpl::runOn(AST* tree)
{
  hlvmAssert(tree && "Null tree?");
  // Call the initializers
  std::vector<Pass*>::iterator PI = all.begin(), PE = all.end();
  while (PI != PE) { (*PI)->handleInitialize(tree); ++PI; }

  // Just a little optimization for empty pass managers
  if (pre.empty() && post.empty())
    return;

  // Traverse each of the bundles in the AST node.
  for (AST::iterator I = tree->begin(), E = tree->end(); I != E; ++I)
    runOn(*I);

  // Call the terminators
  PI = all.begin(), PE = all.end();
  while (PI != PE) { (*PI)->handleTerminate(); ++PI; }
}

void 
PassManagerImpl::runOnNode(Node* startAt)
{
  // Check to make sure startAt is in tree
  hlvmAssert(startAt != 0 && "Can't run passes from null start");

  if (isa<AST>(startAt))
    runOn(cast<AST>(startAt));
  else if (isa<Bundle>(startAt))
    runOn(cast<Bundle>(startAt));
  else if (isa<Block>(startAt))
    runOn(cast<Block>(startAt));
  else if (isa<Linkable>(startAt))
    runOn(cast<Linkable>(startAt));
  else if (isa<ConstantValue>(startAt))
    runOn(cast<ConstantValue>(startAt));
  else if (isa<Operator>(startAt))
    runOn(cast<Operator>(startAt));
  else
    hlvmAssert(!"startAt type not supported");
}

} // anonymous namespace

Pass::~Pass()
{
}

void 
Pass::handleInitialize(AST* tree)
{
}

void 
Pass::handleTerminate()
{
}

PassManager::~PassManager()
{
}

PassManager*
PassManager::create()
{
  return new PassManagerImpl;
}
