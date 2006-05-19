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

namespace hlvm { namespace AST {

Node::~Node()
{
}

NamedNode::~NamedNode()
{
}

bool 
Node::isNamedNode() const
{
  return isType() || isBundle() || isFunction() || isProgram() || isVariable();
}

bool 
Node::isLinkageItem() const
{
  return isFunction() || isType() || isProgram() || isVariable();
}


void 
Node::insertChild(Node* child)
{
  assert(!"This node doesn't accept child nodes");
}

void 
Node::removeChild(Node* child)
{
  assert(!"This node doesn't have child nodes");
}

void 
Node::setParent(Node* p)
{
  if (p == 0)
  {
    parent->removeChild(this);
  }
  parent = p;
  if (p != 0)
  {
    p->insertChild(this);
  }
}

#ifndef _NDEBUG
void 
Node::dump() const 
{
}
#endif

}}
