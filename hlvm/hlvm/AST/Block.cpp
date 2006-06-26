//===-- hlvm/AST/Block.cpp - AST Block Node Class ---------------*- C++ -*-===//
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
/// @file hlvm/AST/Block.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Block.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Block.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/Base/Assert.h>    
#include <hlvm/AST/Linkables.h>    
#include <llvm/Support/Casting.h>

namespace hlvm {

Block::~Block()
{
}

void 
Block::insertChild(Node* child)
{
  hlvmAssert(llvm::isa<Operator>(child));
  MultiOperator::insertChild(child);
  if (llvm::isa<AutoVarOp>(child)) {
    AutoVarOp* av = llvm::cast<AutoVarOp>(child);
    autovars[av->getName()] = av;
  } else if (llvm::isa<ResultOp>(child))
  type = getResultType(); // update type to match type of thing just added
}

void 
Block::removeChild(Node* child)
{
  hlvmAssert(llvm::isa<Operator>(child));
  if (llvm::isa<AutoVarOp>(child)) {
    AutoVarOp* av = llvm::cast<AutoVarOp>(child);
    autovars.erase(av->getName());
  }
  MultiOperator::removeChild(child);
}

AutoVarOp*   
Block::getAutoVar(const std::string& name) const
{
  AutoVarMap::const_iterator I = autovars.find(name);
  if (I == autovars.end())
    return 0;
  return I->second;
}

Block* 
Block::getParentBlock() const
{
  Node* p = getParent();
  while (p && !llvm::isa<Function>(p)) {
    if (llvm::isa<Block>(p))
      return llvm::cast<Block>(p);
    p = p->getParent();
  }
  return 0;
}

ResultOp::~ResultOp() {}
}
