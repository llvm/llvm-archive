//===-- AST Constant Expression Operators -----------------------*- C++ -*-===//
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

/// This class represents an operator that yields a literal constant integer 
/// value.
/// @brief HLVM AST Constant Integer Node
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

/// A constant textual string
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

/// A zero initializer constant. It represents a constant of any type whose
/// entire data is filled with zero bytes.
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

} // end hlvm namespace

#endif
