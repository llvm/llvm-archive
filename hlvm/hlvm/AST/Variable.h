//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Variable.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Variable
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_VARIABLE_H
#define HLVM_AST_VARIABLE_H

#include <hlvm/AST/LinkageItem.h>

namespace hlvm
{
namespace AST
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
      Variable(
        Node* parent, ///< The bundle or function that defines the ariable 
        const std::string& name ///< The name of the variable
      ) : LinkageItem(parent,name) {}
      virtual ~Variable();

    /// @}
    /// @name Data
    /// @{
    protected:
      Type* type_; ///< The type of the variable
    /// @}
  };
} // AST
} // hlvm 
#endif
