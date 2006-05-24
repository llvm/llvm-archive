//===-- AST Function Class --------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Block.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Block
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_BLOCK_H
#define HLVM_AST_BLOCK_H

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This class represents an Variable in the HLVM Abstract Syntax Tree.  
/// A Variable is a storage location of a specific type. It can either be
/// global or local, depending on its parent. Global variables are always
/// contained in a Bundle. Local variables are always contained in a
/// Function.
/// @brief HLVM AST Variable Node
class Block : public Operator
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Operator*> NodeList;
    typedef NodeList::iterator iterator;
    typedef NodeList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  public:
    Block() : Operator(BlockID), ops() {}
    virtual ~Block();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Block*) { return true; }
    static inline bool classof(const Operator* O) { return O->isBlock(); }
    static inline bool classof(const Node* N) { return N->isBlock(); }

  /// @}
  /// @name Iterators
  /// @{
  public:
    iterator       begin()       { return ops.begin(); }
    const_iterator begin() const { return ops.begin(); }
    iterator       end  ()       { return ops.end(); }
    const_iterator end  () const { return ops.end(); }
    size_t         size () const { return ops.size(); }
    bool           empty() const { return ops.empty(); }
    Operator*      front()       { return ops.front(); }
    const Operator*front() const { return ops.front(); }
    Operator*      back()        { return ops.back(); }
    const Operator*back()  const { return ops.back(); }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::vector<Operator*> ops; ///< The operators the Block contains
  /// @}
  friend class AST;
};

} // hlvm 
#endif
