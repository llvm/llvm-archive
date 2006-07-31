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
UniformContainerType::resolveTypeTo(const Type* from, const Type* to)
{
  hlvmAssert(isa<OpaqueType>(from) && !isa<OpaqueType>(to));
  if (elemType == from)
    elemType = to;
}

PointerType::~PointerType() { }
ArrayType::~ArrayType() { }
VectorType::~VectorType() { }
NamedType::~NamedType() {}

void 
NamedType::resolveTypeTo(const Type* from, const Type* to)
{
  hlvmAssert(isa<OpaqueType>(from) && !isa<OpaqueType>(to));
  if (type == from)
    type = to;
}

DisparateContainerType::~DisparateContainerType() { }

const char* 
DisparateContainerType::getPrimitiveName() const
{
  hlvmDeadCode("getPrimitiveName called on a container type");
  return 0;
}

unsigned 
DisparateContainerType::getFieldIndex(const std::string& fldname) const
{
  for (const_iterator I = begin(), E = end(); I != E; ++I )
    if ((*I)->getName() == fldname)
      return I - begin() + 1;
  return 0;
}

void 
DisparateContainerType::resolveTypeTo(const Type* from, const Type* to)
{
  hlvmAssert(isa<OpaqueType>(from) && !isa<OpaqueType>(to));
  for (iterator I = begin(), E = end(); I != E; ++I)
    if ((*I)->getType() == from) {
      (*I)->setType(to);
    }
}


StructureType::~StructureType() { }
ContinuationType::~ContinuationType() { }

SignatureType::~SignatureType() { }
void SignatureType::resolveTypeTo(const Type* from, const Type* to)
{
  DisparateContainerType::resolveTypeTo(from,to);
  if (result == from)
    result = to;
}

}
