//===-- hlvm/AST/Bundle.cpp - AST Bundle Class ------------------*- C++ -*-===//
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
/// @file hlvm/AST/Bundle.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::Bundle.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Type.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/Base/Assert.h>
#include <llvm/Support/Casting.h>

using namespace llvm; 

namespace hlvm {

Bundle::~Bundle() { }

void 
Bundle::insertChild(Node* kid)
{
  hlvmAssert(kid && "Null child!");
  if (Type* Ty = dyn_cast<Type>(kid))
    types.insert(Ty);
  else if (Constant* C = dyn_cast<Constant>(kid)) {
    clist.push_back(C);
    ctable.insert(C);
  } else
    hlvmAssert("Don't know how to insert that in a Bundle");
}

void
Bundle::removeChild(Node* kid)
{
  hlvmAssert(kid && "Null child!");
  if (const Type* Ty = dyn_cast<Type>(kid))
    types.erase(Ty->getName());
  else if (Constant* C = dyn_cast<Constant>(kid)) {
    // This is sucky slow, but we probably won't be removing nodes that much.
    for (clist_iterator I = clist_begin(), E = clist_end(); I != E; ++I )
      if (*I == C) { clist.erase(I); break; }
    ctable.erase(C->getName());
  } else 
    hlvmAssert(!"That node isn't my child");
}

Type*  
Bundle::find_type(const std::string& name) const
{
  if (Node* result = types.lookup(name))
    return llvm::cast<Type>(result);
  return 0;
}

Constant*  
Bundle::find_const(const std::string& name) const
{
  if (Constant* result = ctable.lookup(name))
    return llvm::cast<Constant>(result);
  return 0;
}

Import::~Import()
{
}

}
