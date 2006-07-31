//===-- HLVM AST String Operations ------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/StringOps.h
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Provides the interface to class hlvm::AST::StringOps.
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_STRINGOPS_H
#define HLVM_AST_STRINGOPS_H

#include <hlvm/AST/Operator.h>

namespace hlvm
{

/// Represents an Abstract Syntax Tree node for a string insert operator. This
/// operator provides editing on a string value. It takes three arguments that
/// specify the string to be modified, the location of the insertion, and the
/// string to insert.
/// @brief HLVM AST String Insert Node
class StrInsertOp : public TernaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    StrInsertOp() : TernaryOperator(StrInsertOpID) {}
    virtual ~StrInsertOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const StrInsertOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(StrInsertOpID); }

  /// @}
  friend class AST;
};

/// Represents an Abstract Syntax Tree node for a string insert operator. This
/// operator provides editing on a string value. It takes three arguments that
/// specify the string to be modified, the location of the insertion, and the
/// string to insert.
/// @brief HLVM AST String Insert Node
class StrEraseOp : public TernaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    StrEraseOp() : TernaryOperator(StrEraseOpID) {}
    virtual ~StrEraseOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const StrEraseOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(StrEraseOpID); }

  /// @}
  friend class AST;
};

/// Represents an Abstract Syntax Tree node for a string insert operator. This
/// operator provides editing on a string value. It takes four arguments that
/// specify the string to be modified, the location of the replacement, and the
/// replacement string.
/// @brief HLVM AST String Replace Node
class StrReplaceOp : public MultiOperator
{
  /// @name Constructors
  /// @{
  protected:
    StrReplaceOp() : MultiOperator(StrReplaceOpID) {}
    virtual ~StrReplaceOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const StrReplaceOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(StrReplaceOpID); }

  /// @}
  friend class AST;
};

/// Represents an Abstract Syntax Tree node for a string concatentation 
/// operator. This operator creates a new string that is the concatenation of
/// its two operand strings. 
/// @brief HLVM AST String Concatenation Node
class StrConcatOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    StrConcatOp() : BinaryOperator(StrConcatOpID) {}
    virtual ~StrConcatOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const StrConcatOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(StrConcatOpID); }

  /// @}
  friend class AST;
};

} // hlvm
#endif
