//===-- AST Constant Expression Operators -----------------------*- C++ -*-===//
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
/// @file hlvm/AST/Constants.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/24
/// @since 0.1.0
/// @brief Implements the classes that provide constant expressions
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Constants.h>
#include <hlvm/AST/Type.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

namespace hlvm {

Constant::~Constant() { }
ConstantValue::~ConstantValue() { }
ConstantAny::~ConstantAny() { }
ConstantBoolean::~ConstantBoolean() { }
ConstantCharacter::~ConstantCharacter() { }
ConstantEnumerator::~ConstantEnumerator() { }
ConstantInteger::~ConstantInteger() { }
ConstantReal::~ConstantReal() { }
ConstantString::~ConstantString() { }
ConstantPointer::~ConstantPointer() { }
ConstantAggregate::~ConstantAggregate() { }

void 
ConstantAggregate::insertChild(Node* n)
{
  hlvmAssert(llvm::isa<ConstantValue>(n));
  ConstantValue* CV = llvm::cast<ConstantValue>(n);
  elems.push_back(CV);
}

void 
ConstantAggregate::removeChild(Node* n)
{
  hlvmAssert(llvm::isa<ConstantValue>(n));
  ConstantValue* CV = llvm::cast<ConstantValue>(n);
  for (ElementsList::iterator I = elems.begin(), E = elems.end(); I != E; ++I)
    if (*I == CV) {
      elems.erase(I);
      break;
    }
}

ConstantArray::~ConstantArray() { }
ConstantVector::~ConstantVector() { }
ConstantStructure::~ConstantStructure() { }
ConstantContinuation::~ConstantContinuation() { }

}
