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
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Bundle
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_BUNDLE_H
#define HLVM_AST_BUNDLE_H

#include <hlvm/AST/Node.h>

namespace hlvm { namespace AST {

  class LinkageItem;

  /// This class represents an HLVM Bundle. A Bundle is simply a collection of
  /// declarations and definitions. It is the root of the AST tree and also
  /// the grouping and namespace construct in HLVM. Every compilation unit is
  /// a Bundle. Bundles can also be nested in other Bundles. All programming
  /// constructs are defined as child nodes of some Bundle.
  /// @brief HLVM AST Bundle Node
  class Bundle : public NamedNode
  {
    /// @name Types
    /// @{
    public:
      typedef std::vector<LinkageItem*> NodeList;
      typedef NodeList::iterator iterator;
      typedef NodeList::const_iterator const_iterator;

    /// @}
    /// @name Constructors
    /// @{
    public:
      static Bundle* create(const Locator& location, const std::string& pubid);

    protected:
      Bundle() : NamedNode(BundleID) {}
      virtual ~Bundle();

    /// @}
    /// @name Accessors
    /// @{
    public:
      static inline bool classof(const Bundle*) { return true; }
      static inline bool classof(const Node* N) { return N->isBundle(); }

    /// @}
    /// @name Mutators
    /// @{
    public:
      virtual void insertChild(Node* kid);
      virtual void removeChild(Node* kid);

    /// @}
    /// @name Iterators
    /// @{
    public:
      iterator           begin()       { return kids.begin(); }
      const_iterator     begin() const { return kids.begin(); }
      iterator           end  ()       { return kids.end(); }
      const_iterator     end  () const { return kids.end(); }
      size_t             size () const { return kids.size(); }
      bool               empty() const { return kids.empty(); }
      LinkageItem*       front()       { return kids.front(); }
      const LinkageItem* front() const { return kids.front(); }
      LinkageItem*       back()        { return kids.back(); }
      const LinkageItem* back()  const { return kids.back(); }

    /// @}
    /// @name Data
    /// @{
    protected:
      NodeList    kids;  ///< The vector of children nodes.
    /// @}
    friend class AST;
  };
} // AST
} // hlvm
#endif
