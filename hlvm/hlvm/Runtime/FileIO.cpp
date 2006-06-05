//===-- Runtime File I/O Implementation -------------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/FileIO.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/24
/// @since 0.1.0
/// @brief Implements the functions for runtime file input/output.
//===----------------------------------------------------------------------===//

#include <hlvm/Runtime/FileIO.h>
#include <apr-1/apr_file_io.h>

namespace 
{
}

extern "C" 
{

void* 
hlvm_op_file_open(hlvm_string* uri)
{
  return 0;
}

void 
hlvm_op_file_close(void* fnum)
{
}

uint32_t 
hovm_op_file_write(void* fnum, void* data, size_t len)
{
  return 0;
}

}
