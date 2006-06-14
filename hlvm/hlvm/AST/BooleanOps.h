//===-- AST Boolean Operators Interface -------------------------*- C++ -*-===//
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
/// @file hlvm/AST/BooleanOps.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/14
/// @since 0.2.0
/// @brief Declares the AST Boolean Operators 
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_BOOLEANOPS_H 
#define HLVM_AST_BOOLEANOPS_H 

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This class provides an Abstract Syntax Tree node that represents a not
/// operator. The NotOp is a unary operator that returns false if its operand is
/// is non-zero and true if its operand is zero.
/// @brief AST Not Operator Node   
class NotOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    NotOp() : UnaryOperator(NotOpID)  {}
    virtual ~NotOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const NotOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(NotOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to compute the logical and of two quantities. The AndOp is a binary operator
/// that returns true if both its operands are non-zero and returns false
/// otherwise. 
/// @brief AST And Operator Node   
class AndOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    AndOp() : BinaryOperator(AndOpID)  {}
    virtual ~AndOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const AndOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(AndOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to compute the logical or of two quantities. The OrOp is a binary operator
/// that returns false if both its operands are zero and returns true otherwise.
/// @brief AST Or Operator Node   
class OrOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    OrOp() : BinaryOperator(OrOpID)  {}
    virtual ~OrOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const OrOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(OrOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to compute the logical nor of two quantities. The NorOp is a binary operator
/// that returns true if both its operands are zero and returns false
/// otherwise. 
/// @brief AST And Operator Node   
class NorOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    NorOp() : BinaryOperator(NorOpID)  {}
    virtual ~NorOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const NorOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(NorOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to compute the logical exclusive or of two quantities. The XorOp is a 
/// binary operator that returns true if either of its operands are non-zero 
/// and returns false if both operands are zero or non-zero.
/// @brief AST Xor Operator Node   
class XorOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    XorOp() : BinaryOperator(XorOpID)  {}
    virtual ~XorOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const XorOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(XorOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for performing less-than test. The LessThanOp compares its operands and if 
/// the first operand is less than the second operand it returns true, otherwise
/// it returns false.
/// @brief AST Less-Than Operator Node   
class LessThanOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LessThanOp() : BinaryOperator(LessThanOpID)  {}
    virtual ~LessThanOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const LessThanOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(LessThanOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for performing greater-than test. The GreaterThanOp compares its operands 
/// and if the first operand is greater than the second operand it returns true,
/// otherwise it returns false.
/// @brief AST Less-Than Operator Node   
class GreaterThanOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    GreaterThanOp() : BinaryOperator(GreaterThanOpID)  {}
    virtual ~GreaterThanOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const GreaterThanOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(GreaterThanOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for performing less-equal test. The LessEqualOp compares its operands and 
/// if the first operand is less than or equal to the second operand it returns
/// true, otherwise it returns false.
/// @brief AST Less-Equal Operator Node   
class LessEqualOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LessEqualOp() : BinaryOperator(LessEqualOpID)  {}
    virtual ~LessEqualOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const LessEqualOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(LessEqualOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for performing greater-equal test. The GreaterEqualOp compares its operands
/// and if the first operand is less than or equal to the second operand it 
/// returns true, otherwise it returns false.
/// @brief AST Less-Equal Operator Node   
class GreaterEqualOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    GreaterEqualOp() : BinaryOperator(GreaterEqualOpID)  {}
    virtual ~GreaterEqualOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const GreaterEqualOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(GreaterEqualOpID);}

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for performing equality test. The EqualsOp compares its operands
/// and if the first operand is equal to the second operand it 
/// returns true, otherwise it returns false.
/// @brief AST Equality Operator Node   
class EqualityOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    EqualityOp() : BinaryOperator(EqualityOpID)  {}
    virtual ~EqualityOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const EqualityOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(EqualityOpID);}

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// for performing inequality test. The InequalityOp  compares its operands
/// and if the first operand is less than or equal to the second operand it 
/// returns true, otherwise it returns false.
/// @brief AST Inequality Operator Node   
class InequalityOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    InequalityOp() : BinaryOperator(InequalityOpID)  {}
    virtual ~InequalityOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const InequalityOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(InequalityOpID);}

  /// @}
  friend class AST;
};

} // end hlvm namespace

#endif
