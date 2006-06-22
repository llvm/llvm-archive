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

/// This class provides an Abstract Syntax Tree node for the "noop" operator
/// that does nothing.
/// @brief AST NoOp Operator
class NullOp : public NilaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    NullOp() : NilaryOperator(NullOpID) {}
    virtual ~NullOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const NullOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(NullOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a select 
/// operator. The select operator is a ternary operator that evaluates its first
/// operand as a boolean. If the result is true, the second operand is evaluated
/// and its result is the result of the select operator. If the result of the
/// first operand is false, the third operand is evaluated and its result is the
/// result of the select operator. This is similar to an "if" statement in other
/// languages except it is unrestricted. The three operands can be of any type.
/// @brief AST Select Operator Node
class SelectOp : public TernaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    SelectOp() : TernaryOperator(SelectOpID)  {}
    virtual ~SelectOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const SelectOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(SelectOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a switch
/// statement. THis is one of the more complicated operators in HLVM. The switch
/// operator can have an unlimited number of operands. Its first operand is an
/// expression to evaluate of any simple or primary type. This is the control
/// expression. The remaining operands must be in pairs. The first operand of 
/// the pair is an operator to match agains the control expression. The second
/// operand of the pair is an operator to execute if the first operand of the 
/// pair matched. This is very analagous to a switch statement in other 
/// languages except that it has fewer restrictions. It is not limited to 
/// integer types, but only to those types with a collation order. Its "case 
/// statements" are not limited to constant values but can be full expressions 
/// or even blocks of operators. Of course, constant values will execute faster.
/// @brief AST Switch Operator Node
class SwitchOp : public MultiOperator
{
  /// @name Constructors
  /// @{
  protected:
    SwitchOp() : MultiOperator(SwitchOpID)  {}
    virtual ~SwitchOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const SwitchOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(SwitchOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a
/// generalized loop construct for HLVM. The LoopOp takes three operands, as
/// follows:
/// -# a boolean expression to be evaluated before each iteration begins that 
///    controls loop termination 
/// -# an operator (typically a block) to be evaluated on each iteration of the
///    loop. This is the main body of the loop
/// -# a boolean expression to be evaluated after each iteration ends that 
///    controls loop termination
/// Either or both of the control expressions can be a NoOp operator in which
/// case the test is not performed. If both control expressions are NoOp, the 
/// loop is infinite.
/// @brief AST Loop Operator Node
class LoopOp : public TernaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LoopOp() : TernaryOperator(LoopOpID)  {}
    virtual ~LoopOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const LoopOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(LoopOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a return 
/// operator. The return operator returns from the function that contains it.
/// The ReturnOp takes one operand which is the value to return to the caller
/// of the Function.
/// @brief AST Return Operator Node
class ReturnOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
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

/// This class provides an Abstract Syntax Tree node that represents a break
/// operator. A BreakOp causes control to flow to the operator immediately
/// following the enclosing block. The enclosing block can be any kind of block.
/// If a BreakOp occurs in the main block of a function, it is equivalent to 
/// returning void from that function. If the function doesn't return void, it
/// is an error.
/// @brief AST Break Operator Node
class BreakOp : public NilaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    BreakOp() : NilaryOperator(BreakOpID)  {}
    virtual ~BreakOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const BreakOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(BreakOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a continue
/// operator. A ContinueOp causes control to flow back to the start of the 
/// the enclosing block. The enclosing block is not required to be a loop. If
/// the enclosing block is a loop, continuation first evaluates any exit
/// criteria, then evaluates any entry criteria, and finally re-enters the body
/// of the loop. This ensures that loop termination conditions are checked
/// before continuing.
/// @brief AST Continue Operator Node
class ContinueOp : public NilaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static ContinueOp* create();

  protected:
    ContinueOp() : NilaryOperator(ContinueOpID)  {}
    virtual ~ContinueOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ContinueOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ContinueOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a function
/// call operator. A CallOp invokes a function, possibly passing it arguments,
/// and obtains the result of that function. The value of the CallOp becomes the
/// result of the called function. 
/// @brief AST Call Operator Node
class CallOp : public MultiOperator
{
  /// @name Constructors
  /// @{
  public:
    static CallOp* create();

  protected:
    CallOp() : MultiOperator(CallOpID)  {}
    virtual ~CallOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    /// Returns the type of the value of the call. This is the same as the
    /// result type of the function's signature.
    const Type* getType() const; 
    Function* getCalledFunction() const;
    static inline bool classof(const CallOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(CallOpID); }

  /// @}
  friend class AST;
};

} // end hlvm namespace

#endif
