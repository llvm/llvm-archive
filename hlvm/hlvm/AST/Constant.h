//===-- AST Constant Abstract Class -----------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Constant.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/25
/// @since 0.1.0
/// @brief Declares the AST Constant Class
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_CONSTANT_H
#define HLVM_AST_CONSTANT_H

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
    Constant(NodeIDs id) : Value(id) {}
  public:
    virtual ~Constant();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Constant*) { return true; }
    static inline bool classof(const Node* N) { return N->isConstant(); }

  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif
