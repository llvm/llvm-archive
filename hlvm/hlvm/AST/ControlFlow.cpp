//===-- AST Control Flow Nodes ----------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/ControlFlow.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/24
/// @since 0.1.0
/// @brief Implements the classes that provide program control flow
//===----------------------------------------------------------------------===//

#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

using namespace llvm;

namespace hlvm {

SelectOp::~SelectOp() {}
SwitchOp::~SwitchOp() {}
WhileOp::~WhileOp() {}
const Type* 
WhileOp::getType() const
{
  hlvmAssert(getNumOperands() == 2);
  Operator* op2 = getOperand(1);
  return op2->getType();
}

UnlessOp::~UnlessOp() {}
const Type* 
UnlessOp::getType() const
{
  hlvmAssert(getNumOperands() == 2);
  Operator* op2 = getOperand(1);
  return op2->getType();
}

UntilOp::~UntilOp() {}
const Type* 
UntilOp::getType() const
{
  hlvmAssert(getNumOperands() == 2);
  Operator* op1 = getOperand(0);
  return op1->getType();
}

LoopOp::~LoopOp() {}

const Type* 
LoopOp::getType() const
{
  hlvmAssert(getNumOperands() == 3);
  Operator* op2 = getOperand(1);
  return op2->getType();
}

ReturnOp::~ReturnOp() { }
BreakOp::~BreakOp() {}
ContinueOp::~ContinueOp() {}
CallOp::~CallOp() {}

Function* 
CallOp::getCalledFunction() const
{
  hlvmAssert(isa<GetOp>(getOperand(0)));
  GetOp* refop = cast<GetOp>(getOperand(0));
  const Documentable* referent = refop->getReferent();
  hlvmAssert(isa<Function>(referent));
  return const_cast<Function*>(cast<Function>(referent));
}

const Type* 
CallOp::getType() const 
{
  Function* F = getCalledFunction();
  hlvmAssert(F && "Call With No Function?");
  return F->getSignature()->getResultType();
}

}
