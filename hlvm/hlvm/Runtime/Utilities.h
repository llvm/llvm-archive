//===-- Runtime Utilities Interface -----------------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Utilities.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/05
/// @since 0.1.0
/// @brief Declares the interface to the runtime utilities
//===----------------------------------------------------------------------===//

#ifndef HLVM_RUNTIME_UTILITIES_H
#define HLVM_RUNTIME_UTILITIES_H

/// This is the HLVM runtime assert macro. It is very much similar to
/// the <cassert> version but without some of the overhead. It also lets
/// us take control of what to do when an assertion happens. The standard
/// implementation just prints and aborts.
#define hlvm_assert(expr) \
  (static_cast<void>((expr) ? 0 : \
    (hlvm_assert_fail(" #expr ", __FILE__, __LINE__))))


/// This function gets called by the hlvm_assert macro, and in other situations
/// where a "panic" happens. It provides the proper escape mechanism given the
/// configuration of the runtime.
void hlvm_assert_fail(const char* expression, const char* file, int line_num);


#endif
