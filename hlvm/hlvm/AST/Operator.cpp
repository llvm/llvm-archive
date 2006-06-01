//===-- hlvm/AST/Operator.cpp - AST Operator Class --------------*- C++ -*-===//
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
/// @file hlvm/AST/Operator.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Operator.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Operator.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

using namespace llvm;

namespace hlvm {

Operator::~Operator()
{
}

NilaryOperator::~NilaryOperator()
{
}

Operator*
NilaryOperator::getOperand(unsigned idx) const
{
  hlvmAssert(!"Can't get operands from a NilaryOperator");
  return 0;
}

size_t  
NilaryOperator::numOperands() const
{
  return 0;
}

void 
NilaryOperator::insertChild(Node* child)
{
  hlvmAssert(!"NilaryOperators don't accept children");
}

void 
NilaryOperator::removeChild(Node* child)
{
  hlvmAssert(!"Can't remove from a NilaryOperator");
}

UnaryOperator::~UnaryOperator()
{
}

Operator*
UnaryOperator::getOperand(unsigned idx) const
{
  assert(idx == 0 && "Operand index out of range");
  return op1;
}

size_t  
UnaryOperator::numOperands() const
{
  return op1 != 0;
}

void 
UnaryOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  if (!op1)
    op1 = cast<Operator>(child);
  else
    hlvmAssert("UnaryOperator full");
}

void 
UnaryOperator::removeChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  if (op1 == child) {
    op1 = 0;
  } else
    hlvmAssert(!"Can't remove child from UnaryOperator");
}

BinaryOperator::~BinaryOperator()
{
}

Operator*
BinaryOperator::getOperand(unsigned idx) const
{
  assert(idx <= 1 && "Operand index out of range");
  return ops[idx];
}

size_t  
BinaryOperator::numOperands() const
{
  return (ops[0] ? 1 : 0) + (ops[1] ? 1 : 0);
}

void 
BinaryOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  if (!ops[0])
    ops[0] = cast<Operator>(child);
  else if (!ops[1])
    ops[1] = cast<Operator>(child);
  else
    hlvmAssert(!"BinaryOperator full!");
}

void 
BinaryOperator::removeChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  if (ops[0] == child)
    ops[0] = 0;
  else if (ops[1] == child)
    ops[1] = 0;
  else
    hlvmAssert(!"Can't remove child from BinaryOperator");
}

TernaryOperator::~TernaryOperator()
{
}

Operator*
TernaryOperator::getOperand(unsigned idx) const
{
  assert(idx <= 2 && "Operand index out of range");
  return ops[idx];
}

size_t  
TernaryOperator::numOperands() const
{
  return (ops[0] ? 1 : 0) + (ops[1] ? 1 : 0) + (ops[2] ? 1 : 0);
}

void 
TernaryOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  if (!ops[0])
    ops[0] = cast<Operator>(child);
  else if (!ops[1])
    ops[1] = cast<Operator>(child);
  else if (!ops[2])
    ops[2] = cast<Operator>(child);
  else
    hlvmAssert(!"TernaryOperator full!");
}

void 
TernaryOperator::removeChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  if (ops[0] == child)
    ops[0] = 0;
  else if (ops[1] == child)
    ops[1] = 0;
  else if (ops[2] == child)
    ops[2] = 0;
  else
    hlvmAssert(!"Can't remove child from TernaryOperator!");
}

MultiOperator::~MultiOperator()
{
}

Operator*
MultiOperator::getOperand(unsigned idx) const
{
  assert(idx <= ops.size() && "Operand index out of range");
  return ops[idx];
}

size_t  
MultiOperator::numOperands() const
{
  return ops.size();
}

void 
MultiOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  ops.push_back(cast<Operator>(child));
}

void 
MultiOperator::removeChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  for (iterator I = begin(), E = end(); I != E; ++I ) {
      if (*I == child) { ops.erase(I); return; }
  }
}

}
