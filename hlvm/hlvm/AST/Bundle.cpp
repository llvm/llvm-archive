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
#include <hlvm/AST/Variable.h>
#include <hlvm/AST/Function.h>
#include <hlvm/Base/Assert.h>

using namespace llvm; 

namespace hlvm {

Bundle*
Bundle::create(const Locator* loc, const std::string& id)
{
  Bundle* result = new Bundle();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Bundle::~Bundle()
{
}

void 
Bundle::insertChild(Node* kid)
{
  if (kid->isType())
    types.push_back(cast<Type>(kid));
  else if (kid->isVariable())
    vars.push_back(cast<Variable>(kid));
  else if (kid->isFunction())
    funcs.push_back(cast<Function>(kid));
  else
    hlvmAssert("Don't know how to insert that in a Bundle");
}

void
Bundle::removeChild(Node* kid)
{
  hlvmAssert(isa<LinkageItem>(kid) && "Can't remove that here");
  // This is sucky slow, but we probably won't be removing nodes that much.
  if (kid->isType()) {
    for (type_iterator I = type_begin(), E = type_end(); I != E; ++I ) {
      if (*I == kid) { types.erase(I); return; }
    }
  } else if (kid->isVariable()) {
    for (var_iterator I = var_begin(), E = var_end(); I != E; ++I ) {
      if (*I == kid) { vars.erase(I); return; }
    }
  } else if (kid->isFunction()) {
    for (func_iterator I = func_begin(), E = func_end(); I != E; ++I ) {
      if (*I == kid) { funcs.erase(I); return; }
    }
  }
  hlvmAssert(!"That node isn't my child");
}

}
