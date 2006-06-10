//===-- Type Resolution Pass ------------------------------------*- C++ -*-===//
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
/// @file hlvm/Pass/ResolveTypes.h
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/19
/// @since 0.1.0
/// @brief Declares the class hlvm::Pass::ResolveTypes
//===----------------------------------------------------------------------===//

#ifndef HLVM_PASS_RESOLVETYPES_H
#define HLVM_PASS_RESOLVETYPES_H

#include <hlvm/AST/Pass.h>

namespace hlvm { 
  using namespace AST;
  
namespace Pass {

  /// This class provides a type resolution capability. It searches for 
  /// instances of OpaqueType and resolves all their uses to the correct actual
  /// type.
  /// @brief Type Resolution Pass
  class ResolveTypes : public Pass 
  {
    /// @}
    /// @name Constructors
    /// @{
    protected:
      ResolveTypes() : Pass(0) {}
    public:
      ~ResolveTypes();

    /// @}
    /// @name Handlers
    /// @{
    public:
      /// Handle a Block node. Subclasses should override this; default 
      /// implementation does nothing.
      virtual void handle(Block* b);

      /// Handle a Bundle node. Subclasses should override this; default 
      /// implementation does nothing.
      virtual void handle(Bundle* b);

      /// Handle a Function node. Subclasses should override this; default 
      /// implementation does nothing.
      virtual void handle(Function* f);

      /// Handle a Program node. Subclasses should override this; default 
      /// implementation does nothing.
      virtual void handle(Program* p);

      /// Handle a Operator node. Subclasses should override this; default 
      /// implementation does nothing.
      virtual void handle(Operator* o);

      /// Handle a Type node. Subclasses should override this; default 
      /// implementation does nothing.
      virtual void handle(Type* t);

    /// @}
    /// @name Data
    /// @{
    protected:
      int interest;
    /// @}
  };
} // AST
} // hlvm
#endif
