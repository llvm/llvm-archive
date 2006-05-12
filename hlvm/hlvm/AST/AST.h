//===-- hlvm/AST/AST.h - AST Container Class --------------------*- C++ -*-===//
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
/// @file hlvm/AST/AST.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::AST
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_AST_H
#define HLVM_AST_AST_H

#include <hlvm/AST/Node.h>

/// This namespace is for all HLVM software. It ensures that HLVM software does
/// not collide with any other software. Hopefully HLVM is not a namespace used
/// elsewhere. 
namespace hlvm
{
/// This namespace contains all the AST (Abstract Syntax Tree) module code. All
/// node types of the AST are declared in this namespace.
namespace AST
{
  /// This class is used to hold or contain an Abstract Syntax Tree. It provides
  /// those aspects of the tree that are not part of the tree itself.
  /// @brief AST Container Class
  class AST
  {
    /// @name Constructors
    /// @{
    public:
      AST() : tree_(0) {}
      virtual ~AST();
#ifndef _NDEBUG
      virtual void dump() const;
#endif

    /// @}
    /// @name Accessors
    /// @{
    public:

    /// @}
    /// @name Data
    /// @{
    protected:
      Node* tree_;
    /// @}
  };
} // AST
} // hlvm
#endif
