//===-- AST SymbolTable Class -----------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/SymbolTable.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::SymbolTable
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_SYMBOLTABLE_H
#define HLVM_AST_SYMBOLTABLE_H

#include <map>

namespace hlvm 
{

/// This class provides a symbol table of name/node pairs with operations to
/// support constructing, searching and iterating over the symbol table.
template<class ElemType>
class SymbolTable
{
/// @name Types
/// @{
public:
  /// @brief A mapping of names to nodes.
  typedef typename std::map<const std::string, const ElemType*> NodeMap;

  /// @brief An iterator over the NodeMap.
  typedef typename NodeMap::iterator iterator;

  /// @brief A const_iterator over the NodeMap.
  typedef typename NodeMap::const_iterator const_iterator;

/// @}
/// @name Constructors
/// @{
public:
  SymbolTable() {}
  ~SymbolTable() {}

/// @}
/// @name Accessors
/// @{
public:
  /// Generates a unique name for a node based on the \p BaseName by
  /// incrementing an integer and appending it to the name, if necessary
  /// @returns the unique name
  /// @brief Get a unique name for a node
  std::string getUniqueName(const std::string &BaseName) const;

  /// This method finds the node with the given \p name in the node map
  /// and returns it.
  /// @returns null if the name is not found, otherwise the ElemType
  /// associated with the \p name.
  /// @brief Lookup a node by name.
  ElemType* lookup(const std::string& name) const;

  /// @returns true iff the symbol table is empty.
  /// @brief Determine if the symbol table is empty
  inline bool empty() const { return map_.empty(); }

  /// @returns the size of the symbol table
  /// @brief The number of name/node pairs is returned.
  inline unsigned size() const { return unsigned(map_.size()); }

/// @}
/// @name Iteration
/// @{
public:
  /// Get an iterator to the start of the symbol table
  inline iterator begin() { return map_.begin(); }

  /// @brief Get a const_iterator to the start of the symbol table
  inline const_iterator begin() const { return map_.begin(); }

  /// Get an iterator to the end of the symbol talbe. 
  inline iterator end() { return map_.end(); }

  /// Get a const_iterator to the end of the symbol table.
  inline const_iterator end() const { return map_.end(); }

/// @}
/// @name Mutators
/// @{
public:
  /// Inserts a node into the symbol table with the specified name. There can
  /// be a many-to-one mapping between names and nodes. This method allows a 
  /// node with an existing entry in the symbol table to get a new name.
  /// @brief Insert a node under a new name.
  void insert(const std::string &Name, const ElemType *N);

  /// Remove a node at the specified position in the symbol table.
  /// @returns the removed ElemType.
  /// @returns the ElemType that was erased from the symbol table.
  ElemType* erase(iterator TI);

  /// Remove a node using a specific key
  bool erase(const std::string& name) { return map_.erase(name) > 0; }

  /// Remove a specific ElemType from the symbol table. This isn't fast, linear
  /// search, O(n), algorithm.
  /// @returns true if the erase was successful (TI was found)
  bool erase(ElemType* TI);

  /// Rename a node. This ain't fast, we have to linearly search for it first.
  /// @returns true if the rename was successful (node was found)
  bool rename(ElemType* T, const std::string& new_name);

/// @}
/// @name Internal Data
/// @{
private:
  NodeMap map_; ///< This is the mapping of names to types.
  mutable unsigned long last_unique_; ///< Counter for tracking unique names
/// @}
};

} // End hlvm namespace
#endif
