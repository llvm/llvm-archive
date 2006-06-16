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

#include <hlvm/AST/Constant.h>

namespace hlvm
{

/// This enumeration is used to specify the kinds of linkage that are
/// permitted for a LinkageItem.
/// @brief Enumeration of ways to link bundles
enum LinkageKinds {
  ExternalLinkage   = 1, ///< Externally visible item
  LinkOnceLinkage   = 2, ///< Keep one copy of item when linking (inline)
  WeakLinkage       = 3, ///< Keep one copy of item when linking (weak)
  AppendingLinkage  = 4, ///< Append item to an array of similar items
  InternalLinkage   = 5  ///< Rename collisions when linking (static funcs)
};

/// This class provides an Abstract Syntax Tree node that represents an item
/// which can be linked with other Bundles. LinkageItem is an abstract base 
/// class and cannot be instantiated. All LinkageItem's are Constant values
/// because they represents a runtime value that is a constant address. The
/// value pointed to by the LinkageItem may be mutable or immutable depending
/// on its type and options.  As the name suggests, LinkageItems participate
/// in linkage. A Bundle referring to a name in another Bundle will only link
/// with a LinkageItem and nothing else. There are several ways in which 
/// LinkageItems can be linked together, specified by the LinkageKinds value.
/// @see LinkageKinds
/// @see Bundle
/// @see Constant
/// @brief AST Bundle Node
class LinkageItem : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    LinkageItem( NodeIDs id ) : Constant(id), name() {
      setLinkageKind(InternalLinkage);
    }
  public:
    virtual ~LinkageItem();

  /// @}
  /// @name Accessors
  /// @{
  public:
    inline const std::string& getName() const { return name; }
    inline LinkageKinds getLinkageKind() const { 
      return LinkageKinds(flags & 0x0007); }
    static inline bool classof(const LinkageItem*) { return true; }
    static inline bool classof(const Node* N) { return N->isLinkageItem(); }

  /// @}
  /// @name Mutators
  /// @{
    void setName(const std::string& n) { name = n; }
    void setLinkageKind(LinkageKinds k) { 
      flags &= 0xFFF8; flags |= uint16_t(k); 
    }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;
  /// @}
  friend class AST;
};

} // hlvm
#endif
