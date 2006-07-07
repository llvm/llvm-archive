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
#include "hlvm/AST/Type.h"
#include "hlvm/AST/Linkables.h"
#include "hlvm/Base/Assert.h"
#include "llvm/ADT/StringExtras.h"
#include <iostream>
#include <algorithm>

namespace hlvm {

// lookup a node by name - returns null on failure
template<class ElemType>
ElemType* SymbolTable<ElemType>::lookup(const std::string& name) const {
  const_iterator TI = map_.find(&name);
  if (TI != map_.end())
    return const_cast<ElemType*>(TI->second);
  return 0;
}
 
template<class ElemType> bool 
SymbolTable<ElemType>::erase(const std::string& name) 
{
  iterator I = map_.find(&name);
  if (I == map_.end())
    return false;
  map_.erase(I);
  return true;
}

// Erase a specific type from the symbol table
template<class ElemType>
bool SymbolTable<ElemType>::erase(ElemType *N) {
  for (iterator TI = map_.begin(), TE = map_.end(); TI != TE; ++TI) {
    if (TI->second == N) {
      this->erase(TI);
      return true;
    }
  }
  return false;
}

// remove - Remove a node from the symbol table...
template<class ElemType>
ElemType* SymbolTable<ElemType>::erase(iterator Entry) {
  hlvmAssert(Entry != map_.end() && "Invalid entry to remove!");
  const ElemType* Result = Entry->second;
  map_.erase(Entry);
  return const_cast<ElemType*>(Result);
}

// insert - Insert a node into the symbol table with the specified name...
template<class ElemType>
void SymbolTable<ElemType>::insert(ElemType* N) {
  hlvmAssert(N && "Can't insert null node into symbol table!");

  // It is an error to reuse a name
  hlvmAssert(0 == lookup(N->getName()));

  // Insert the map entry
  map_.insert(make_pair(&N->getName(), N));
}

/// rename - Given a value with a non-empty name, remove its existing entry
/// from the symbol table and insert a new one for Name.  This is equivalent to
/// doing "remove(V), V->Name = Name, insert(V)", but is faster, and will not
/// temporarily remove the symbol table plane if V is the last value in the
/// symtab with that name (which could invalidate iterators to that plane).
template<class ElemType>
bool SymbolTable<ElemType>::rename(ElemType *N, const std::string &name) {
  iterator TI = map_.find(&name);
  if (TI != map_.end()) {
    map_.erase(TI);
    N->setName(name);
    this->insert(N);
    return true;
  }
  return false;
}

// instantiate for Types and Linkabes
template class SymbolTable<Type>;
template class SymbolTable<Constant>;

}
