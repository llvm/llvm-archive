//===-- HLVM AST Memory Operators -------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/MemoryOps.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Implements the various AST memory operators
//===----------------------------------------------------------------------===//

#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Constants.h>
#include <llvm/Support/Casting.h>

namespace hlvm {

LoadOp::~LoadOp() {}
StoreOp::~StoreOp() {}
AutoVarOp::~AutoVarOp() {}

GetOp::~GetOp() {}

const Type*
GetOp::getReferentType() const
{
  const Value* ref = getReferent();
  if (const AutoVarOp* avo = llvm::dyn_cast<AutoVarOp>(ref))
    return avo->getType();
  else if (const Variable* var = llvm::dyn_cast<Variable>(ref))
    return var->getType();
  else if (const Function* func = llvm::dyn_cast<Function>(ref))
    return func->getResultType();
  else if (const Constant* cnst = llvm::dyn_cast<Constant>(ref))
    return cnst->getType();
  return ref->getType();
}

GetFieldOp::~GetFieldOp() {}

const Type* 
GetFieldOp::getFieldType() const
{
  if (const DisparateContainerType* DCTy = 
      llvm::dyn_cast<DisparateContainerType>(op1->getType()))
    return DCTy->getFieldType(fieldName);
  return 0;
}

GetIndexOp::~GetIndexOp() {}

const Type* 
GetIndexOp::getIndexedType() const
{
  if (const UniformContainerType* UCTy =
      llvm::dyn_cast<UniformContainerType>(ops[0]->getType()))
    return UCTy->getElementType();
  return 0;
}

}
