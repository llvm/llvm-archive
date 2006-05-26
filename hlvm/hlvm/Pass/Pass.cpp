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
#include <hlvm/AST/Program.h>
#include <hlvm/AST/Variable.h>

using namespace hlvm;
using namespace llvm;

namespace {

class PassManagerImpl : public PassManager
{
public:
  PassManagerImpl() : PassManager(), pre(), post() {}
  void addPass(Pass* p);
  void runOn(AST* tree);

  inline void runIfInterested(Pass* p, Node* n, Pass::TraversalKinds m);
  inline void runPreOrder(Node* n);
  inline void runPostOrder(Node* n);
  inline void runOn(Operator* b);
  inline void runOn(Block* b);
  inline void runOn(Bundle* b);

private:
  std::vector<Pass*> pre;
  std::vector<Pass*> post;
};

void
PassManagerImpl::addPass(Pass* p)
{
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
     ((interest & Pass::Block_Interest) && n->isBlock()) ||
     ((interest & Pass::Operator_Interest) && n->isOperator()) ||
     ((interest & Pass::Program_Interest) && n->isProgram()) ||
     ((interest & Pass::Variable_Interest) && n->isVariable())
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

inline void
PassManagerImpl::runOn(Operator* op)
{
  runPreOrder(op);
  size_t limit = op->numOperands();
  for (size_t i = 0; i < limit; ++i) {
    runOn(op->getOperand(i));
  }
  runPostOrder(op);
}

inline void
PassManagerImpl::runOn(Block* b)
{
  runPreOrder(b);
  for (Block::iterator I = b->begin(), E = b->end(); I != E; ++I) {
    if (!*I)
      break;
    if (isa<Block>(*I))
      runOn(cast<Block>(*I)); // recurse!
    else
      runOn(cast<Operator>(*I)); 
  }
  runPostOrder(b);
}

inline void 
PassManagerImpl::runOn(Bundle* b)
{
  runPreOrder(b);
  for (Bundle::type_iterator TI =b->type_begin(), TE = b->type_end(); 
       TI != TE; ++TI) {
    runPreOrder(*TI);
    runPostOrder(*TI);
  }
  for (Bundle::var_iterator VI = b->var_begin(), VE = b->var_end(); 
       VI != VE; ++VI) {
    runPreOrder(*VI);
    runPostOrder(*VI);
  }
  for (Bundle::func_iterator FI = b->func_begin(), FE = b->func_end(); 
       FI != FE; ++FI) {
    runPreOrder(*FI);
    runOn((*FI)->getBlock());
    runPostOrder(*FI);
  }
  runPostOrder(b);
}

void PassManagerImpl::runOn(AST* tree)
{
  // Just a little optimization for empty pass managers
  if (pre.empty() && post.empty())
    return;

  // Traverse each of the bundles in the AST node.
  for (AST::iterator I = tree->begin(), E = tree->end(); I != E; ++I)
    runOn(*I);
}

} // anonymous namespace

Pass::~Pass()
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
