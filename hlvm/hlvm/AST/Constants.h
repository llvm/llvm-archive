//===-- AST Constant Values -------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Constants.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/25
/// @since 0.1.0
/// @brief Declares the AST Constant Expression Operators 
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_CONSTANTS_H
#define HLVM_AST_CONSTANTS_H

#include <hlvm/AST/Constant.h>

namespace hlvm 
{

/// This class provides an Abstract Syntax Tree node that yields a 
/// constant integer value. This kind of constant can represent integer valued
/// constants of any of the signed or unsigned integer types of any bitsize.
/// @see IntegerType
/// @brief AST Constant Integer Node
class ConstantInteger: public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantInteger() : Constant(ConstantIntegerID) {}
    virtual ~ConstantInteger();

  /// @}
  /// @name Accessors
  /// @{
  public:
    uint64_t getValue(int) const { return value.u; }
    int64_t  getValue()    const { return value.s; }
    static inline bool classof(const ConstantInteger*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantIntegerID); }

  /// @}
  /// @name Accessors
  /// @{
  public:
    void setValue(uint64_t v) { value.u = v; }
    void setValue(int64_t v)  { value.s = v; }

  /// @}
  /// @name Data
  /// @{
  public:
    union {
      uint64_t u;
      int64_t  s;
    } value;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that yields a constant
/// real number value. This kind of constant can represent a constant real
/// number value of any mantissa or exponent size. 
/// @see RealType
/// @brief AST Constant Real Node
class ConstantReal : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantReal() : Constant(ConstantRealID)  {}
    virtual ~ConstantReal();

  /// @}
  /// @name Accessors
  /// @{
  public:
    double getValue() const { return value; }
    static inline bool classof(const ConstantReal*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantRealID); }

  /// @}
  /// @name Accessors
  /// @{
  public:
    void setValue(double v ) { value = v; }

  /// @}
  /// @name Data
  /// @{
  public:
    double value;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that yields a constant 
/// text value. The constant value is encoded in UTF-8 but may be converted 
/// to other encodings depending on how it is used. For example, when used to
/// initialize a TextType Variable that is using UTF-16 encoding, the conversion
/// will occur whenever the Variable is loaded. UTF-8 encoding is used for
/// constant text values to reduce storage requirements and for compatibility
/// with older non-Unicode systems.
/// @see TextType
/// @brief AST Constant Text Node
class ConstantText : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantText() : Constant(ConstantTextID)  {}
    virtual ~ConstantText();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string&  getValue() const{ return value; }
    static inline bool classof(const ConstantText*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantTextID); }

  /// @}
  /// @name Accessors
  /// @{
  public:
    void setValue(const std::string& v ) { value = v; }

  /// @}
  /// @name Data
  /// @{
  public:
    std::string value;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that yields a zero value
/// for any type. In essence, this is a short-cut. Zero value constants are very
/// common and defining them explicitly for each type of constant makes use of
/// the AST constants cumbersome. The way to think about this node is that it
/// represents a constant value of any type such that all the bits of that type
/// are zero.
/// @brief AST Constant Zero Node
class ConstantZero : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantZero() : Constant(ConstantZeroID)  {}
    virtual ~ConstantZero();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ConstantZero*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantZeroID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that yields a constant
/// aggregate. This can be used to specify constant values of aggregate types
/// such as arrays, vectors, structures and continuations. It simply contains 
/// a list of other elements which themselves must be constants.
/// @brief AST Constant Array Node.
class ConstantAggregate : public Constant
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Constant*> ElementsList;
    typedef ElementsList::iterator iterator;
    typedef ElementsList::const_iterator const_iterator;

  /// @name Constructors
  /// @{
  protected:
    ConstantAggregate() : Constant(ConstantAggregateID), elems()  {}
    virtual ~ConstantAggregate();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ConstantAggregate*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantAggregateID); }

  /// @}
  /// @name Mutators
  /// @{
  protected:
    virtual void insertChild(Node* n);
    virtual void removeChild(Node* n);

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator         begin()       { return elems.begin(); }
    const_iterator   begin() const { return elems.begin(); }
    iterator         end  ()       { return elems.end(); }
    const_iterator   end  () const { return elems.end(); }
    size_t           size () const { return elems.size(); }
    bool             empty() const { return elems.empty(); }
    Constant*        front()       { return elems.front(); }
    const Constant*  front() const { return elems.front(); }
    Constant*        back()        { return elems.back(); }
    const Constant*  back()  const { return elems.back(); }

  /// @}
  /// @name Data
  /// @{
  protected:
    ElementsList elems; ///< The contained types
  /// @}

  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that yields a constant
/// expression. The expression uses a limited set of operator identifiers that
/// can yield constants such as arithmetic or comparison operators. 
/// @brief AST Constant Expression Node.
class ConstantExpression : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantExpression(NodeIDs exprOp) : Constant(ConstantExpressionID)
      { flags = exprOp; }
    virtual ~ConstantExpression();
  public:

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ConstantExpression*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantExpressionID); }

  /// @}
  friend class AST;
};

} // end hlvm namespace

#endif
