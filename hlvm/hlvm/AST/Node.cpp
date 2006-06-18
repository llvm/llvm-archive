//===-- hlvm/AST/Node.cpp - AST Abstract Node Class -------------*- C++ -*-===//
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
/// @file hlvm/AST/Node.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Node.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Node.h>
#include <hlvm/AST/AST.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

namespace hlvm {

Node::~Node()
{
}

AST*
Node::getRoot()
{
  Node* p = parent, *last = this; 
  while (p!=0) { 
    last = p; 
    p = p->parent; 
  }
  return llvm::cast<AST>(last);
}

void 
Node::insertChild(Node* child)
{
  hlvmNotImplemented("Node::insertChild");
}

void 
Node::removeChild(Node* child)
{
  hlvmNotImplemented("Node::insertChild");
}

void 
Node::setParent(Node* p)
{
  if (parent != 0)
  {
    parent->removeChild(this);
  }
  parent = p;
  if (parent != 0)
  {
    parent->insertChild(this);
  }
}

Documentation::~Documentation()
{
}

Documentable::~Documentable()
{
}

Value::~Value()
{
}

}
