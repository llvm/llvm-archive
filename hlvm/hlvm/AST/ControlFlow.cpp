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

NullOp::~NullOp() {}
SelectOp::~SelectOp() {}
SwitchOp::~SwitchOp() {}
LoopOp::~LoopOp() {}
ReturnOp::~ReturnOp() { }
BreakOp::~BreakOp() {}
ContinueOp::~ContinueOp() {}
CallOp::~CallOp() {}

Function* 
CallOp::getCalledFunction() const
{
  hlvmAssert(isa<ReferenceOp>(getOperand(0)));
  ReferenceOp* refop = cast<ReferenceOp>(getOperand(0));
  const Value* referent = refop->getReferent();
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
