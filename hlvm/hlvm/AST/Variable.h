//===-- AST Variable Class --------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Variable.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Variable
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_VARIABLE_H
#define HLVM_AST_VARIABLE_H

#include <hlvm/AST/LinkageItem.h>

namespace hlvm 
{

class Type; // Forward declare

/// This class represents an Variable in the HLVM Abstract Syntax Tree.  
/// A Variable is a storage location of a specific type. It can either be
/// global or local, depending on its parent. Global variables are always
/// contained in a Bundle. Local variables are always contained in a
/// Function.
/// @brief HLVM AST Variable Node
class Variable : public LinkageItem
{
  /// @name Constructors
  /// @{
  public:
    static Variable* create(const Locator& loc, std::string name);
  protected:
    Variable() : LinkageItem(VariableID) {}
  public:
    virtual ~Variable();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const Type* getType() const { return type; }
    static inline bool classof(const Variable*) { return true; }
    static inline bool classof(const Node* N) { return N->isVariable(); }

  /// @}
  /// @name Accessors
  /// @{
  public:
    void setType(const Type* t) { type = t; }

  /// @}
  /// @name Data
  /// @{
  protected:
    const Type* type; ///< The type of the variable
  /// @}
  friend class AST;
};

} // hlvm 
#endif
