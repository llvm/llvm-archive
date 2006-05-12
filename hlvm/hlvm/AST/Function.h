//===-- hlvm/AST/Function.h - AST Function Class ----------------*- C++ -*-===//
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
/// @file hlvm/AST/Function.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Function
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_FUNCTION_H
#define HLVM_AST_FUNCTION_H

#include <hlvm/AST/LinkageItem.h>

namespace hlvm
{
namespace AST
{
  // Forward declarations
  class Block; 
  class SignatureType;  

  /// This class represents a Function in the HLVM Abstract Syntax Tree.  
  /// A Function is a callable block of code that accepts parameters and 
  /// returns a result.  This is the basic unit of code in HLVM. A Function
  /// has a name, a set of formal arguments, a return type, and a block of
  /// code to execute.
  /// @brief HLVM AST Function Node
  class Function : public LinkageItem
  {
    /// @name Constructors
    /// @{
    public:
      Function(
        SignatureType* sig, ///< The function signature
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name, ///< The name of the function
        NodeIDs id = FunctionID
      ) : LinkageItem(id,parent,name), block_(0), signature_(sig) {}
      virtual ~Function();

    /// @}
    /// @name Accessors
    /// @{
    public:
      static inline bool classof(const Function*) { return true; }
      static inline bool classof(const Node* N) { return N->isFunction(); }

    /// @}
    /// @name Data
    /// @{
    protected:
      Block * block_;                   ///< The code block to be executed
      SignatureType* signature_;        ///< The function signature.
    /// @}
  };
} // AST
} // hlvm
#endif
