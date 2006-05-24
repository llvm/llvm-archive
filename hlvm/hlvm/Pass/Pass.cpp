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

namespace {

class PassManagerImpl : public PassManager
{
public:
  PassManagerImpl() : PassManager(), begins(), ends(0) {}
  void addPass(Pass* p);
  void runOn(AST* tree);

  inline void runIfInterested(Pass* p, Node* n, Pass::PassMode m);
  inline void runBegins(Node* n);
  inline void runEnds(Node* n);
  inline void runOn(Bundle* b);

private:
  std::vector<Pass*> begins;
  std::vector<Pass*> ends;
  Pass* bfsFirst; // First pass for breadth first traversal
};

void
PassManagerImpl::addPass(Pass* p)
{
  if (p->mode() & Pass::Begin_Mode)
    begins.push_back(p);
  if (p->mode() & Pass::End_Mode)
    ends.push_back(p);
}

inline void
PassManagerImpl::runIfInterested(Pass* p, Node* n, Pass::PassMode m)
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
PassManagerImpl::runBegins(Node* n)
{
  std::vector<Pass*>::iterator I = begins.begin(), E = begins.end();
  while (I != E) {
    runIfInterested(*I,n,Pass::Begin_Mode);
    ++I;
  }
}

inline void 
PassManagerImpl::runEnds(Node* n)
{
  std::vector<Pass*>::iterator I = ends.begin(), E = ends.end();
  while (I != E) {
    runIfInterested(*I,n,Pass::End_Mode);
    ++I;
  }
}

inline void 
PassManagerImpl::runOn(Bundle* b)
{
  runBegins(b);
  for (Bundle::type_iterator I=b->type_begin(), E=b->type_end(); I != E; ++I) {
    runBegins((*I));
    runEnds((*I));
  }
  for (Bundle::var_iterator I=b->var_begin(), E=b->var_end(); I != E; ++I) {
    runBegins((*I));
    runEnds((*I));
  }
  for (Bundle::func_iterator I=b->func_begin(), E=b->func_end(); I != E; ++I) {
    runBegins((*I));
    runEnds((*I));
  }
  runEnds(b);
}

void PassManagerImpl::runOn(AST* tree)
{
  if (begins.empty() && ends.empty())
    return;

  for (AST::iterator I = tree->begin(), E = tree->end(); I != E; ++I)
  {
    runOn(*I);
  }
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
