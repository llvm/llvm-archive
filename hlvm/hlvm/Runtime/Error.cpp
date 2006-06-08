//===-- Runtime Error Handler Implementation --------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Error.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/06/07
/// @since 0.1.0
/// @brief Implements the runtime error handling facilities.
//===----------------------------------------------------------------------===//

extern "C" {

#include <apr-1/apr_file_io.h>
#include <hlvm/Runtime/Error.h>
#include <hlvm/Runtime/Internal.h>
#include <stdlib.h>

void 
hlvm_error(ErrorCodes ec, const char* arg)
{
  const char* msg = "Unknown";
  switch (ec) {
    case E_UNHANDLED_EXCEPTION: msg = "Unhandled Exception"; break;
    case E_BAD_OPTION         : msg = "Unrecognized option"; break;
    case E_MISSING_ARGUMENT   : msg = "Missing option argument"; break;
    case E_NO_PROGRAM_NAME    : msg = "Program name not specified"; break;
    case E_PROGRAM_NOT_FOUND  : msg = "Program not found"; break;
    default: break;
  }
  if (arg)
    apr_file_printf(_hlvm_stderr, "Error: %s: %s\n", msg, arg);
  else
    apr_file_printf(_hlvm_stderr, "Error: %s\n", msg);
}

void hlvm_panic(const char* msg) {
  exit(254);
}

}
