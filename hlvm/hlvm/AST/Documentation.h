//===-- AST Documentation Classes -------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Documentation.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/19
/// @since 0.1.0
/// @brief Declares the class hlvm::Documentation
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_DOCUMENTATION_H
#define HLVM_AST_DOCUMENTATION_H

#include <hlvm/AST/Node.h>

namespace hlvm 
{

/// The HLVM AST permits documentation (not just comments) to be included into
/// the nodes of the AST. Each such block of documentation is represented by
/// a Documentation node, implemented by this class. The content of a 
/// documentation node is simply a block of text. The intended use is that
/// the text contain XHTML markup. In this way, an automated documentation
/// facility can translate the AST into XHTML documentation with perfect
/// precision. Since the documentation node can be associated with any kind
/// of node, this affords a complete system for documenting HLVM programs 
/// with XHTML markup.
/// @brief HLVM AST Function Node
class Documentation : public Node
{
  /// @name Constructors
  /// @{
  protected:
    Documentation() : Node(DocumentationID) {}

  public:
    virtual ~Documentation();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getDoc() const { return doc; }
    static inline bool classof(const Documentation*) { return true; }
    static inline bool classof(const Node* N) 
    { return N->is(DocumentationID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setDoc(const std::string& d) { doc = d; }
    void addDoc(const std::string& d) { doc += d; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string doc;
  /// @}
  friend class AST;
};

} // hlvm
#endif
