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
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

using namespace llvm;

namespace hlvm {

Operator::~Operator()
{
}

Function* 
Operator::getContainingFunction()
{
  Node* p = getParent();
  while (p && !p->isFunction()) p = p->getParent();
  if (!p)
    return 0;
  return cast<Function>(p);
}

Block*
Operator::getContainingBlock()
{
  Node* p = getParent();
  while (p && !isa<Block>(p)) p = p->getParent();
  if (!p)
    return 0;
  return cast<Block>(p);
}

Operator*
Operator::getContainingLoop()
{
  Node* p = getParent();
  while (p && !p->isLoop()) p = p->getParent();
  if (!p)
    return 0;
  return cast<Operator>(p);
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
NilaryOperator::getNumOperands() const
{
  return 0;
}

void
NilaryOperator::setOperand(unsigned opnum, Operator* operand)
{
  hlvmAssert(!"Can't set operands on a NilaryOperator!");
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
UnaryOperator::getNumOperands() const
{
  return op1 != 0;
}

void
UnaryOperator::setOperand(unsigned opnum, Operator* operand)
{
  hlvmAssert(opnum == 0 && "Operand Index out of range for UnaryOperator!");
  operand->setParent(this);
}

void 
UnaryOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  hlvmAssert(child != op1 && "Re-insertion of child");
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
BinaryOperator::getNumOperands() const
{
  return (ops[0] ? 1 : 0) + (ops[1] ? 1 : 0);
}

void
BinaryOperator::setOperand(unsigned opnum, Operator* operand)
{
  hlvmAssert(opnum <= 1 && "Operand Index out of range for BinaryOperator!");
  operand->setParent(this);
}

void 
BinaryOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  hlvmAssert(child != ops[0] && child != ops[1] && "Re-insertion of child");
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
TernaryOperator::getNumOperands() const
{
  return (ops[0] ? 1 : 0) + (ops[1] ? 1 : 0) + (ops[2] ? 1 : 0);
}

void
TernaryOperator::setOperand(unsigned opnum, Operator* operand)
{
  hlvmAssert(opnum <= 2 && "Operand Index out of range for TernaryOperator!");
  operand->setParent(this);
}

void 
TernaryOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
  hlvmAssert(child != ops[0] && child != ops[1] && child != ops[2] && 
             "Re-insertion of child");
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
MultiOperator::getNumOperands() const
{
  return ops.size();
}

void
MultiOperator::setOperand(unsigned opnum, Operator* operand)
{
  if (ops.capacity() < opnum + 1)
    ops.resize(opnum+1);
  operand->setParent(this);
}

void 
MultiOperator::addOperands(const OprndList& oprnds) 
{
  for (const_iterator I = oprnds.begin(), E = oprnds.end(); I != E; ++I )
  {
    hlvmAssert(isa<Operator>(*I));
    (*I)->setParent(this);
  }
}

void 
MultiOperator::insertChild(Node* child)
{
  hlvmAssert(isa<Operator>(child));
#ifdef HLVM_ASSERT
  for (const_iterator I = begin(), E = end(); I != E; ++I)
    hlvmAssert((*I) != child && "Re-insertion of child");
#endif
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
