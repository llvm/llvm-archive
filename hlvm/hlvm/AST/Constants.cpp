//===-- AST Constant Expression Operators -----------------------*- C++ -*-===//
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
/// @file hlvm/AST/Constants.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/24
/// @since 0.1.0
/// @brief Implements the classes that provide constant expressions
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Constants.h>
#include <hlvm/AST/Type.h>

namespace hlvm {

ConstLiteralInteger::~ConstLiteralInteger()
{
}

ConstLiteralInteger* 
ConstLiteralInteger::create()
{
  return new ConstLiteralInteger;
}

ConstLiteralString::~ConstLiteralString()
{
}

ConstLiteralString* 
ConstLiteralString::create()
{
  return new ConstLiteralString;
}

}
