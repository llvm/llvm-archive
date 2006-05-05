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
/// @file hlvm/AST/LinkageItem.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::LinkageItem
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_LINKAGEITEM_H
#define HLVM_AST_LINKAGEITEM_H

#include <hlvm/AST/Node.h>

namespace hlvm
{
namespace AST
{
  /// This enumeration is used to specify the kinds of linkage that are
  /// permitted for a LinkageItem.
  enum LinkageTypes {
    ExternalLinkage,    ///< Externally visible item
    InternalLinkage     ///< Rename collisions when linking (static funcs)
    LinkOnceLinkage,    ///< Keep one copy of item when linking (inline)
    WeakLinkage,        ///< Keep one copy of item when linking (weak)
    AppendingLinkage,   ///< Append item to an array of similar items
  };

  /// This class represents an LinkageItem in the HLVM Abstract Syntax Tree. 
  /// A LinkageItem is any construct that can be linked; that is, referred to
  /// elsewhere and linked into another bundle to resolve the reference. The
  /// LinkageItem declares what kind of linkage is to be performed.
  /// @brief HLVM AST Bundle Node
  class LinkageItem : Node
  {
    /// @name Constructors
    /// @{
    public:
      LinkageItem(
        Bundle* parent, ///< The Bundle to which this bundle belongs, or null
        const std::string& name ///< The name of the bundle
      ) : Node(parent,name) {}
      virtual ~Bundle();

    /// @}
    /// @name Data
    /// @{
    protected:
      LinkageTypes type_; ///< The type of linkage to perform for this item
    /// @}
  };
}

#endif

#endif
