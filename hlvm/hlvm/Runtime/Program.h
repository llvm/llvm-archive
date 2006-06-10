//===-- Runtime Program Interface -------------------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Program.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/24
/// @since 0.1.0
/// @brief Declares the interface to the runtime program facilities
//===----------------------------------------------------------------------===//

#ifndef HLVM_RUNTIME_PROGRAM_H
#define HLVM_RUNTIME_PROGRAM_H

/// This is the type of a program entry point function.
typedef int (*hlvm_program_type)(int, const char **);

/// This is the type of the appending array of program entry elements
struct hlvm_programs_element {
  const char* program_name;
  hlvm_program_type program_entry;
};

/// This function searches the hlvm_programs array for an entry matching
/// uri and returns a pointer to the corresponding program.
extern hlvm_program_type hlvm_find_program(const char* uri);

#endif
