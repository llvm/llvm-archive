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

#include <hlvm/AST/Node.h>

namespace hlvm 
{

/// This abstract base class represents a constant value in the HLVM Abstract 
/// Syntax Tree.  All Constants are immutable values of a specific type. 
/// Constants do not have a storage location nor an address nor do they
/// participate in linking.  However, as they are values they may be used as 
/// the operand of instructions or as the initializers of variables. Constants
/// do not participate in linking and are always internal to the bundle in which
/// they appear. To create a linkable constant, declare a variable that is 
/// constant and initialize it with a Constant.  There are many kinds of 
/// constants including simple literal values (numbers an text), complex 
/// constant expressions (constant computations), and aggregate constants that
/// represent constant arrays, vectors, pointers and structures.
/// @see hlvm/AST/Constants.h
/// @brief AST Abstract Constant Node
class Constant : public Value
{
  /// @name Constructors
  /// @{
  protected:
    Constant(NodeIDs id) : Value(id), name()  {}
    virtual ~Constant();

  /// @}
  /// @name Accessors
  /// @{
  public:
    // Get the type of the value
    inline const std::string& getName() const { return name; }
    static inline bool classof(const Constant*) { return true; }
    static inline bool classof(const Node* N) { return N->isConstant(); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setName(const std::string& n) { name = n; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name; ///< The name of this Value.
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that yields a 
/// constant boolean value. 
/// @brief AST Constant Boolean Node
class ConstantBoolean: public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantBoolean(bool val) : Constant(ConstantBooleanID) {
      flags = val; }
    virtual ~ConstantBoolean();

  /// @}
  /// @name Accessors
  /// @{
  public:
    bool getValue() const { return flags != 0; }
    static inline bool classof(const ConstantBoolean*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantBooleanID); }

  /// @}
  friend class AST;
};

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
    ConstantInteger(uint16_t base) : Constant(ConstantIntegerID) {}
    virtual ~ConstantInteger();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getValue() const { return value; }
    uint16_t getBase() const { return flags; }
    static inline bool classof(const ConstantInteger*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantIntegerID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setValue(const std::string& v) { value = v; }
    void setBase(uint16_t base) { flags = base; }

  /// @}
  /// @name Data
  /// @{
  public:
    std::string value;
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
    const std::string& getValue() const { return value; }
    static inline bool classof(const ConstantReal*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantRealID); }

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

/// This class provides an Abstract Syntax Tree node that yields a constant 
/// string value. The constant value is encoded in UTF-8 with a null terminator.
/// @see StringType
/// @brief AST Constant String Node
class ConstantString : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    ConstantString() : Constant(ConstantStringID)  {}
    virtual ~ConstantString();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string&  getValue() const{ return value; }
    static inline bool classof(const ConstantString*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstantStringID); }

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

  /// @}
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
