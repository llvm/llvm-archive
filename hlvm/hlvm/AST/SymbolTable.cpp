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
/// @file hlvm/AST/SymbolTable.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::SymbolTable.
//===----------------------------------------------------------------------===//

#include "hlvm/AST/SymbolTable.h"
#include "llvm/ADT/StringExtras.h"
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace hlvm::AST;

std::string 
SymbolTable::getUniqueName(const std::string &base_name) const {
  std::string try_name = base_name;
  const_iterator end = map_.end();

  // See if the name exists. Loop until we find a free name in the symbol table
  // by incrementing the last_unique_ counter.
  while (map_.find(try_name) != end)
    try_name = base_name + 
      llvm::utostr(++last_unique_);
  return try_name;
}

// lookup a node by name - returns null on failure
Node* SymbolTable::lookup(const std::string& name) const {
  const_iterator TI = map_.find(name);
  if (TI != map_.end())
    return const_cast<Node*>(TI->second);
  return 0;
}

// Erase a specific type from the symbol table
bool SymbolTable::erase(Node *N) {
  for (iterator TI = map_.begin(), TE = map_.end(); TI != TE; ++TI) {
    if (TI->second == N) {
      this->erase(TI);
      return true;
    }
  }
  return false;
}

// remove - Remove a node from the symbol table...
Node* SymbolTable::erase(iterator Entry) {
  assert(Entry != map_.end() && "Invalid entry to remove!");
  const Node* Result = Entry->second;
  map_.erase(Entry);
  return const_cast<Node*>(Result);
}

// insert - Insert a node into the symbol table with the specified name...
void SymbolTable::insert(const std::string& Name, const Node* N) {
  assert(N && "Can't insert null node into symbol table!");

  // Check to see if there is a naming conflict.  If so, rename this type!
  std::string unique_name = Name;
  if (lookup(Name))
    unique_name = getUniqueName(Name);

  // Insert the map entry
  map_.insert(make_pair(unique_name, N));
}

/// rename - Given a value with a non-empty name, remove its existing entry
/// from the symbol table and insert a new one for Name.  This is equivalent to
/// doing "remove(V), V->Name = Name, insert(V)", but is faster, and will not
/// temporarily remove the symbol table plane if V is the last value in the
/// symtab with that name (which could invalidate iterators to that plane).
bool SymbolTable::rename(Node *N, const std::string &name) {
  for (iterator NI = map_.begin(), NE = map_.end(); NI != NE; ++NI) {
    if (NI->second == N) {
      // Remove the old entry.
      map_.erase(NI);
      // Add the new entry.
      this->insert(name,N);
      return true;
    }
  }
  return false;
}

#ifndef _NDEBUG
static void DumpNodes(const std::pair<const std::string, const Node*>& I ) {
  std::cerr << "  '" << I.first << "' = ";
  I.second->dump();
  std::cerr << "\n";
}

void SymbolTable::dump() const {
  std::cerr << "SymbolTable: ";
  for_each(map_.begin(), map_.end(), DumpNodes);
}
#endif
