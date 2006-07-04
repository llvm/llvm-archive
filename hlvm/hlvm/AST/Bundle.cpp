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
  if (isa<Type>(kid))
    types.insert(cast<Type>(kid)->getName(), cast<Type>(kid));
  else if (Constant* C = dyn_cast<Constant>(kid)) {
    const std::string& name = C->getName();
    values.push_back(C);
    if (isa<ConstantValue>(kid)) {
      cvals.insert(name, cast<ConstantValue>(kid));
    } else if (isa<Linkable>(kid)) {
      linkables.insert(name, cast<Linkable>(kid));
    }
  } else
    hlvmAssert("Don't know how to insert that in a Bundle");
}

void
Bundle::removeChild(Node* kid)
{
  hlvmAssert(kid && "Null child!");
  if (isa<Type>(kid))
    types.erase(cast<Type>(kid)->getName());
  else if (isa<Value>(kid)) {
    // This is sucky slow, but we probably won't be removing nodes that much.
    for (value_iterator I = value_begin(), E = value_end(); I != E; ++I )
      if (*I == kid) { values.erase(I); break; }
    if (isa<ConstantValue>(kid))
      cvals.erase(cast<ConstantValue>(kid)->getName());
    else if (isa<Linkable>(kid))
      linkables.erase(cast<Linkable>(kid)->getName());
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

ConstantValue*  
Bundle::find_cval(const std::string& name) const
{
  if (Node* result = cvals.lookup(name))
    return llvm::cast<ConstantValue>(result);
  return 0;
}

Linkable*  
Bundle::find_linkable(const std::string& name) const
{
  if (Node* result = linkables.lookup(name))
    return llvm::cast<Linkable>(result);
  return 0;
}

Import::~Import()
{
}

}
