//===-- hlvm/AST/ContainerType.cpp - AST ContainerType Class ----*- C++ -*-===//
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
/// @file hlvm/AST/ContainerType.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Implements the functions of the various AST container types
//===----------------------------------------------------------------------===//

#include <hlvm/AST/ContainerType.h>
#include <hlvm/Base/Assert.h>
#include <llvm/ADT/StringExtras.h>

using namespace llvm;

namespace hlvm {

UniformContainerType::~UniformContainerType()
{
}

const char* 
UniformContainerType::getPrimitiveName() const
{
  hlvmDeadCode("getPrimitiveName called on a container type");
  return 0;
}

void 
UniformContainerType::insertChild(Node* n)
{
  hlvmAssert(isa<Type>(n) && "Can't insert those here");
  if (type)
    const_cast<Type*>(type)->setParent(0);
  type = cast<Type>(n);
}

void 
UniformContainerType::removeChild(Node* n)
{
  hlvmAssert(isa<Type>(n) && "Can't remove those here");
  hlvmAssert(n->getParent() == this && "Node isn't my kid!");
  hlvmAssert(type == n && "Node isn't mine");
  type = 0;
}

PointerType::~PointerType()
{
}

ArrayType::~ArrayType()
{
}

VectorType::~VectorType()
{
}

AliasType::~AliasType()
{
}

const char*
AliasType::getPrimitiveName() const
{
  return type->getPrimitiveName();
}

DisparateContainerType::~DisparateContainerType()
{
}

const char* 
DisparateContainerType::getPrimitiveName() const
{
  hlvmDeadCode("getPrimitiveName called on a container type");
  return 0;
}

void 
DisparateContainerType::insertChild(Node* n)
{
  hlvmAssert(isa<AliasType>(n) && "Can't insert those here");
  contents.push_back(cast<AliasType>(n));
}

void 
DisparateContainerType::removeChild(Node* n)
{
  hlvmAssert(isa<Type>(n) && "Can't remove those here");
  // This is sucky slow, but we probably won't be removing nodes that much.
  for (iterator I = begin(), E = end(); I != E; ++I ) {
    if (*I == n) { contents.erase(I); break; }
  }
  hlvmAssert(!"That node isn't my child");
}

StructureType::~StructureType()
{
}

SignatureType::~SignatureType()
{
}

}
