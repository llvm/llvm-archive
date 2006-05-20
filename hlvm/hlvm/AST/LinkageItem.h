//===-- AST LinkageItem Class -----------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/LinkageItem.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::LinkageItem
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_LINKAGEITEM_H
#define HLVM_AST_LINKAGEITEM_H

#include <hlvm/AST/Node.h>

namespace hlvm
{

/// This enumeration is used to specify the kinds of linkage that are
/// permitted for a LinkageItem.
enum LinkageTypes {
  ExternalLinkage,    ///< Externally visible item
  InternalLinkage,    ///< Rename collisions when linking (static funcs)
  LinkOnceLinkage,    ///< Keep one copy of item when linking (inline)
  WeakLinkage,        ///< Keep one copy of item when linking (weak)
  AppendingLinkage    ///< Append item to an array of similar items
};

/// This class represents an LinkageItem in the HLVM Abstract Syntax Tree. 
/// A LinkageItem is any construct that can be linked; that is, referred to
/// elsewhere and linked into another bundle to resolve the reference. The
/// LinkageItem declares what kind of linkage is to be performed.
/// @brief HLVM AST Bundle Node
class LinkageItem : public NamedNode
{
  /// @name Constructors
  /// @{
  protected:
    LinkageItem(
      NodeIDs id ///< Subclass's node identifier
    ) : NamedNode(id) {}
  public:
    virtual ~LinkageItem();

  /// @}
  /// @name Mutators
  /// @{
  public:
    static inline bool classof(const LinkageItem*) { return true; }
    static inline bool classof(const Node* N) { return N->isLinkageItem(); }
  /// @}
  /// @name Mutators
  /// @{

  /// @}
  /// @name Data
  /// @{
  protected:
    LinkageTypes type_; ///< The type of linkage to perform for this item
  /// @}
  friend class AST;
};

} // hlvm
#endif
