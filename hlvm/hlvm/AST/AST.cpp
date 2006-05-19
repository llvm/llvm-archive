//===-- hlvm/AST/AST.cpp - AST Container Class ------------------*- C++ -*-===//
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
/// @file hlvm/AST/AST.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the functions of class hlvm::AST::AST.
//===----------------------------------------------------------------------===//

#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Function.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/Variable.h>

namespace hlvm {
namespace AST {

Type* 
AST::resolveType(const std::string& name) const
{
  IntegerType* result = new IntegerType();
  result->setName(name);
  return result;
}

Bundle*
AST::new_Bundle(const Locator& loc, const std::string& id)
{
  Bundle* result = new Bundle();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Function*
AST::new_Function(const Locator& loc, const std::string& id)
{
  Function* result = new Function();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Import*
AST::new_Import(const Locator& loc, const std::string& pfx)
{
  Import* result = new Import();
  result->setLocator(loc);
  result->setPrefix(pfx);
  return result;
}

SignatureType*
AST::new_SignatureType(const Locator& loc, const std::string& id)
{
  SignatureType* result = new SignatureType();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

Variable*
AST::new_Variable(const Locator& loc, const std::string& id)
{
  Variable* result = new Variable();
  result->setLocator(loc);
  result->setName(id);
  return result;
}

}}
