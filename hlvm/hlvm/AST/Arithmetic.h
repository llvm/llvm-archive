//===-- AST Arithemetic Operators Interface ---------------------*- C++ -*-===//
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
/// @file hlvm/AST/Arithmetic.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/14
/// @since 0.2.0
/// @brief Declares the AST Arithmentic Operators 
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_ARITHMETIC_H 
#define HLVM_AST_ARITHMETIC_H 

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This class provides an Abstract Syntax Tree node that represents a negation
/// operator. The NegateOpID is a unary operator that negates its operand and
/// returns that value. 
/// @brief AST Negate Operator Node   
class NegateOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    NegateOp() : UnaryOperator(NegateOpID)  {}
    virtual ~NegateOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const NegateOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(NegateOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a
/// complement operator. The ComplementOpID is a unary operator that complements
/// its operand and returns that value. 
/// @brief AST Complement Operator Node   
class ComplementOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    ComplementOp() : UnaryOperator(ComplementOpID)  {}
    virtual ~ComplementOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ComplementOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ComplementOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a
/// pre-increment operator. The PreIncrOpID is a unary operator that increments
/// its operand and returns the incremented value. 
/// @brief AST Pre-Increment Operator Node   
class PreIncrOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    PreIncrOp() : UnaryOperator(PreIncrOpID)  {}
    virtual ~PreIncrOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const PreIncrOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(PreIncrOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a 
/// post-increment operator. The PostIncrOpID is a unary operator that returns
/// its operand and arranges for that operand to be incremented some time after
/// the value has been used. 
/// @brief AST Post-Increment Operator Node   
class PostIncrOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    PostIncrOp() : UnaryOperator(PostIncrOpID)  {}
    virtual ~PostIncrOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const PostIncrOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(PostIncrOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a 
/// pre-decrement operator. The PreDecrOp is a unary operator that decrements 
/// the value of its operand and returns that decremented value.
/// @brief AST Pre-Decrement Operator Node   
class PreDecrOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    PreDecrOp() : UnaryOperator(PreDecrOpID)  {}
    virtual ~PreDecrOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const PreDecrOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(PreDecrOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a 
/// post-decrement operator. The PostDecrOp is a unary operator that 
/// returns the value of its operand and then arranges for that value to be 
/// decremented after it has been used.
/// @brief AST Post-Decrement Operator Node   
class PostDecrOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    PostDecrOp() : UnaryOperator(PostDecrOpID)  {}
    virtual ~PostDecrOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const PostDecrOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(PostDecrOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to add two quantities. The AddOp is a binary operator that
/// computes the sum of its two operands and returns that value.
/// @brief AST Add Operator Node   
class AddOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    AddOp() : BinaryOperator(AddOpID)  {}
    virtual ~AddOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const AddOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(AddOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to subtract two quantities. The SubractOp is a binary operator that
/// computes the difference of its two operands and returns that value.
/// @brief AST Subtract Operator Node   
class SubtractOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    SubtractOp() : BinaryOperator(SubtractOpID)  {}
    virtual ~SubtractOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const SubtractOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(SubtractOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to multiply two quantities. The MultiplyOp is a binary operator that
/// computes the product of its two operands and returns that value.
/// @brief AST Multiply Operator Node   
class MultiplyOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    MultiplyOp() : BinaryOperator(MultiplyOpID)  {}
    virtual ~MultiplyOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const MultiplyOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(MultiplyOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an operator
/// to divide two quantities. The DivideOp is a binary operator that
/// computes the dividend of its two operands and returns that value.
/// @brief AST Divide Operator Node   
class DivideOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    DivideOp() : BinaryOperator(DivideOpID)  {}
    virtual ~DivideOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const DivideOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(DivideOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a modulo
/// operator. The ModuloOp is a binary operator that computes the remainder 
/// when its operands are divided and returns that value.
/// @brief AST Modulo Operator Node   
class ModuloOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    ModuloOp() : BinaryOperator(ModuloOpID)  {}
    virtual ~ModuloOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ModuloOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ModuloOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a bitwise
/// and operator. The BAndOp is a binary operator that computes a bitwise and on
/// its two operands and returns that value. BAndOp can only be used with
/// integral types. 
/// @brief AST Bitwise And Operator Node   
class BAndOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    BAndOp() : BinaryOperator(BAndOpID)  {}
    virtual ~BAndOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const BAndOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(BAndOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a bitwise
/// or operator.  The BOrOp is a binary operator that computes the bitwise or
/// of its two operands and returns that value. 
/// @brief AST Bitwise Or Operator Node   
class BOrOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    BOrOp() : BinaryOperator(BOrOpID)  {}
    virtual ~BOrOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const BOrOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(BOrOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a bitwise
/// exclusive or operator. The BXorOp is a binary operator that computes the
/// bitwise exclusive or of its two operands and returns that value.
/// @brief AST Bitwise Exclusive Or Operator Node   
class BXorOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    BXorOp() : BinaryOperator(BXorOpID)  {}
    virtual ~BXorOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const BXorOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(BXorOpID); }

  /// @}
  friend class AST;
};

} // end hlvm namespace

#endif
