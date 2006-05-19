//===-- hlvm/AST/Bundle.h - AST Bundle Class --------------------*- C++ -*-===//
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
/// @file hlvm/AST/Bundle.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Bundle
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_BUNDLE_H
#define HLVM_AST_BUNDLE_H

#include <hlvm/AST/Node.h>

namespace hlvm {
namespace AST {
  /// This class represents an HLVM Bundle. A Bundle is simply a collection of
  /// declarations and definitions. It is the root of the AST tree and also
  /// the grouping and namespace construct in HLVM. Every compilation unit is
  /// a Bundle. Bundles can also be nested in other Bundles. All programming
  /// constructs are defined as child nodes of some Bundle.
  /// @brief HLVM AST Bundle Node
  class Bundle : public ParentNode
  {
    /// @name Constructors
    /// @{
    public:
      static Bundle* create(const Locator& location, const std::string& pubid);

    protected:
      Bundle() : ParentNode(BundleID) {}
      virtual ~Bundle();

    /// @}
    /// @name Accessors
    /// @{
    public:
      static inline bool classof(const Bundle*) { return true; }
      static inline bool classof(const Node* N) { return N->isBundle(); }

    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
    friend class AST;
  };
} // AST
} // hlvm
#endif
