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

namespace hlvm {

Operator::~Operator()
{
}

NilaryOperator::~NilaryOperator()
{
}

Operator*
NilaryOperator::getOperand(unsigned idx)
{
  assert(!"Can't get operands from a NilaryOperator");
}

UnaryOperator::~UnaryOperator()
{
}

Operator*
UnaryOperator::getOperand(unsigned idx)
{
  assert(idx == 0 && "Operand index out of range");
  return op1;
}

BinaryOperator::~BinaryOperator()
{
}

Operator*
BinaryOperator::getOperand(unsigned idx)
{
  assert(idx <= 1 && "Operand index out of range");
  return ops[idx];
}

TernaryOperator::~TernaryOperator()
{
}

Operator*
TernaryOperator::getOperand(unsigned idx)
{
  assert(idx <= 2 && "Operand index out of range");
  return ops[idx];
}

MultiOperator::~MultiOperator()
{
}

Operator*
MultiOperator::getOperand(unsigned idx)
{
  assert(idx <= ops.size() && "Operand index out of range");
  return ops[idx];
}

}
