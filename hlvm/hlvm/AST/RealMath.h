//===-- AST Real Math Operators Interface -----------------------*- C++ -*-===//
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
/// @file hlvm/AST/RealMath.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/14
/// @since 0.2.0
/// @brief Declares the AST Real Math Operators 
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_REALMATH_H
#define HLVM_AST_REALMATH_H

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This class provides an Abstract Syntax Tree node that represents a test
/// for positive infinity on a real value.  The IsPInOp  is a unary operator 
/// that determines if its real number typed operand is the postive infinity 
/// value or not. A Boolean value is returned.
/// @brief AST Positive Infinity Test Operator Node   
class IsPInfOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    IsPInfOp() : UnaryOperator(IsPInfOpID)  {}
    virtual ~IsPInfOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const IsPInfOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(IsPInfOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a test
/// for negative infinity on a real value.  The IsNInOp  is a unary operator 
/// that determines if its real number typed operand is the negative infinity 
/// value or not. A Boolean value is returned.
/// @brief AST Negative Infinity Test Operator Node   
class IsNInfOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    IsNInfOp() : UnaryOperator(IsNInfOpID)  {}
    virtual ~IsNInfOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const IsNInfOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(IsNInfOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a test
/// for not-a-number (NaN) on a real value.  The IsNaNOp  is a unary operator 
/// that determines if its real number typed operand is the not-a-number
/// value or not. A Boolean value is returned.
/// @brief AST Not-A-Number Test Operator Node   
class IsNanOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    IsNanOp() : UnaryOperator(IsNanOpID)  {}
    virtual ~IsNanOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const IsNanOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(IsNanOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a real 
/// number truncation operator. The TruncOp removes the fractional portion of
/// its real number operand and returns that truncated value.
/// @brief AST Truncate Operator Node   
class TruncOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    TruncOp() : UnaryOperator(TruncOpID)  {}
    virtual ~TruncOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const TruncOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(TruncOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a real 
/// number rounding operator. The TruncOp rounds its real number operand towards
/// the nearest integer, but rounds halfway cases away from zero. The rounded
/// value is returned. This follows the C99 standard for rounding floating point
/// numbers.
/// @brief AST Rounding Operator Node   
class RoundOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    RoundOp() : UnaryOperator(RoundOpID)  {}
    virtual ~RoundOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const RoundOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(RoundOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a real 
/// number rounding operator. The FloorOp rounds its real number down to the
/// nearest integer. 
/// @brief AST Floor Operator Node   
class FloorOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    FloorOp() : UnaryOperator(FloorOpID)  {}
    virtual ~FloorOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const FloorOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(FloorOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a real 
/// number rounding operator. The CeilingOp rounds its real number operand 
/// upwards to the nearest integer. The rounded value is returned.
/// @brief AST Ceiling Operator Node   
class CeilingOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    CeilingOp() : UnaryOperator(CeilingOpID)  {}
    virtual ~CeilingOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const CeilingOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(CeilingOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an 
/// natural logarithm operator.  The LogEOp is a unary operator that computes 
/// the natural (base e, Euler's number) logarithm of its operand and returns
/// the result.
/// @brief AST Natural Logarithm Operator Node   
class LogEOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LogEOp() : UnaryOperator(LogEOpID)  {}
    virtual ~LogEOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const LogEOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(LogEOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a base 
/// two logarithm operator.  The Log2Op is a unary operator that computes 
/// the base 2 logaritm of its operand and returns the result. 
/// @brief AST Base 2 Logarithm Operator Node   
class Log2Op : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    Log2Op() : UnaryOperator(Log2OpID)  {}
    virtual ~Log2Op();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Log2Op*) { return true; }
    static inline bool classof(const Node* N) { return N->is(Log2OpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a base 
/// ten logarithm operator.  The Log10Op is a unary operator that computes 
/// the base 10 logaritm of its operand and returns the result. 
/// @brief AST Base 10 Logarithm Operator Node   
class Log10Op : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    Log10Op() : UnaryOperator(Log10OpID)  {}
    virtual ~Log10Op();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Log10Op*) { return true; }
    static inline bool classof(const Node* N) { return N->is(Log10OpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a square 
/// root operator.  The SquareRootOp is a unary operator that computes 
/// the square root of its operand and returns the result. 
/// @brief AST Squar Root Operator Node   
class SquareRootOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    SquareRootOp() : UnaryOperator(SquareRootOpID)  {}
    virtual ~SquareRootOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const SquareRootOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(SquareRootOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a cube 
/// root operator.  The CubeRootOp is a unary operator that computes 
/// the cube root of its operand and returns the result. 
/// @brief AST Squar Root Operator Node   
class CubeRootOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    CubeRootOp() : UnaryOperator(CubeRootOpID)  {}
    virtual ~CubeRootOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const CubeRootOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(CubeRootOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a factorial
/// operator.  The FactorialOp is a unary operator that computes 
/// the factorial of its operand and returns the result. 
/// @brief AST Factorial Operator Node   
class FactorialOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    FactorialOp() : UnaryOperator(FactorialOpID)  {}
    virtual ~FactorialOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const FactorialOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(FactorialOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an power
/// operator.  The PowerOp is a binary operator that raises the value of its
/// first operand to the power of its second operand and returns the result.
/// @brief AST Power Operator Node   
class PowerOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    PowerOp() : BinaryOperator(PowerOpID)  {}
    virtual ~PowerOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const PowerOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(PowerOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an root
/// operator.  The RootOp is a binary operator that determines the root of its
/// first operand as a base of its second operand and returns the result.
/// @brief AST Root Operator Node   
class RootOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    RootOp() : BinaryOperator(RootOpID)  {}
    virtual ~RootOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const RootOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(RootOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a greatest
/// common denominator operator.  The GCDOp is a unary operator that computes 
/// the greatest common denominator of its operands and returns the result. 
/// @brief AST Factorial Operator Node   
class GCDOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    GCDOp() : BinaryOperator(GCDOpID)  {}
    virtual ~GCDOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const GCDOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(GCDOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a least
/// common multiple operator. The LCMOp is a binary operator that computes the
/// least common multiple of its operands and returns the result.
/// @brief AST Least Common Multiple Operator Node   
class LCMOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    LCMOp() : BinaryOperator(LCMOpID)  {}
    virtual ~LCMOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const LCMOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(LCMOpID); }

  /// @}
  friend class AST;
};

} // end hlvm namespace

#endif
