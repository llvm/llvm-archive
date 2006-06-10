//===-- AST Control Flow Operators ------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/ControlFlow.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/25
/// @since 0.1.0
/// @brief Declares the AST Control flow classes (Loop, If, Call, Return, etc.)
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_CONTROLFLOW_H
#define HLVM_AST_CONTROLFLOW_H

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This class provides an Abstract Syntax Tree node that represents a return 
/// operator. The return operator returns from the function that contains it.
/// The ReturnOp takes one operand which is the value to return to the caller
/// of the Function.
/// @brief AST Return Operator Node
class ReturnOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static ReturnOp* create();

  protected:
    ReturnOp() : UnaryOperator(ReturnOpID)  {}
    virtual ~ReturnOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    Value* getResult() { return UnaryOperator::op1; }
    static inline bool classof(const ReturnOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ReturnOpID); }

  /// @}
  /// @name Accessors
  /// @{
  public:
    void setResult(Value* op) { op->setParent(this); }
  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif
