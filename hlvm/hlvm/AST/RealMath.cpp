//===-- AST Real Math Operators Implementation ------------------*- C++ -*-===//
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
/// @file hlvm/AST/RealMath.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/14
/// @since 0.2.0
/// @brief Implements the AST Real Math Operators 
//===----------------------------------------------------------------------===//

#include <hlvm/AST/RealMath.h>

namespace hlvm {

IsPInfOp::~IsPInfOp() {}
IsNInfOp::~IsNInfOp() {}
IsNanOp::~IsNanOp() {}
TruncOp::~TruncOp() {}
RoundOp::~RoundOp() {}
FloorOp::~FloorOp() {}
CeilingOp::~CeilingOp() {}
LogEOp::~LogEOp() {}
Log2Op::~Log2Op() {}
Log10Op::~Log10Op() {}
SquareRootOp::~SquareRootOp() {}
CubeRootOp::~CubeRootOp() {}
FactorialOp::~FactorialOp() {}
PowerOp::~PowerOp() {}
RootOp::~RootOp() {}
GCDOp::~GCDOp() {}
LCMOp::~LCMOp() {}

} // end hlvm namespace
