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

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This class represents an operator that yields a literal constant integer 
/// value.
/// @brief HLVM AST Constant Integer Node
class ConstLiteralInteger : public NilaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static ConstLiteralInteger* create();

  protected:
    ConstLiteralInteger() : NilaryOperator(ConstLiteralIntegerOpID) {}
    virtual ~ConstLiteralInteger();

  /// @}
  /// @name Accessors
  /// @{
  public:
    uint64_t getValue(int) const { return value.u; }
    int64_t  getValue()    const { return value.s; }
    static inline bool classof(const ConstLiteralInteger*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstLiteralIntegerOpID); }

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

class ConstLiteralString : public NilaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static ConstLiteralString* create();

  protected:
    ConstLiteralString() : NilaryOperator(ConstLiteralStringOpID)  {}
    virtual ~ConstLiteralString();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string&  getValue() { return value; }
    static inline bool classof(const ConstLiteralString*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(ConstLiteralStringOpID); }

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

} // hlvm 
#endif
