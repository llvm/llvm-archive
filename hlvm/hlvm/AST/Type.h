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
/// @file hlvm/AST/Type.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Type
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_TYPE_H
#define HLVM_AST_TYPE_H

#include <hlvm/AST/Node.h>

namespace hlvm
{
namespace AST
{
  /// This class represents a Type in the HLVM Abstract Syntax Tree.  
  /// A Type defines the format of storage. 
  /// @brief HLVM AST Type Node
  class Type : public Node
  {
    /// @name Constructors
    /// @{
    public:
      Type(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : Node(parent,name) {}
      virtual ~Type();

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
