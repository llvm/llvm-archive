//===-- AST Arithemetic Operators Implementation ----------------*- C++ -*-===//
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
/// @file hlvm/AST/Arithmetic.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/14
/// @since 0.2.0
/// @brief Implements the AST Arithmentic Operators 
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Arithmetic.h>

namespace hlvm 
{

NegateOp::~NegateOp() {}
ComplementOp::~ComplementOp() {}
PreIncrOp::~PreIncrOp() {}
PostIncrOp::~PostIncrOp() {}
PreDecrOp::~PreDecrOp() {}
PostDecrOp::~PostDecrOp() {}
AddOp::~AddOp() {}
SubtractOp::~SubtractOp() {}
MultiplyOp::~MultiplyOp() {}
DivideOp::~DivideOp() {}
ModuloOp::~ModuloOp() {}
BAndOp::~BAndOp() {}
BOrOp::~BOrOp() {}
BXorOp::~BXorOp() {}
BNorOp::~BNorOp() {}

} // end hlvm namespace
