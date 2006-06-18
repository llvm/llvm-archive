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

/// Represents the String insert operator.
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
  /// @name Mutators
  /// @{
  public:

  /// @}
  /// @name Data
  /// @{
  protected:
  /// @}
  friend class AST;
};

} // hlvm
#endif
namespace hlvm {

}
