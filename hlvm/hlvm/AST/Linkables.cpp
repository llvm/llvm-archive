//===-- AST Linkables Implementation ----------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Linkables.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the subclasses of Linkable
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Block.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

using namespace llvm;

namespace hlvm {

Linkable::~Linkable() { }
Variable::~Variable() { }
Function::~Function() { }
Program::~Program() { }

const SignatureType* 
Function::getSignature() const 
{
  return cast<SignatureType>(type); 
}

void 
Function::insertChild(Node* kid)
{
  if (isa<Block>(kid)) {
    if (block)
      block->setParent(0);
    block = cast<Block>(kid);
  } else {
    hlvmAssert(!"Can't insert one of those here");
  }
}

void 
Function::removeChild(Node* kid)
{
  if (isa<Block>(kid) && kid == block) {
    block = 0;
  } else {
    hlvmAssert(!"Can't remove one of those here");
  }
}

}
