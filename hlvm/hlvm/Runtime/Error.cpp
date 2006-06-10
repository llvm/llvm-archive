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

#include <hlvm/Runtime/Internal.h>

extern "C" {

#include <hlvm/Runtime/Error.h>
#include <stdlib.h>

void
hlvm_verror(ErrorCodes ec, va_list ap)
{
  const char* format = "Unknown Error";
  switch (ec) {
    case E_UNHANDLED_EXCEPTION: format = "Unhandled Exception"; break;
    case E_BAD_OPTION         : format = "Unrecognized option: %s"; break;
    case E_MISSING_ARGUMENT   : format = "Missing option argument for %s";break;
    case E_NO_PROGRAM_NAME    : format = "Program name not specified"; break;
    case E_PROGRAM_NOT_FOUND  : format = "Program '%s' not found"; break;
    case E_APR_ERROR          : format = "%s while %s"; break;
    case E_ASSERT_FAIL        : format = "Assertion Failure: (%s) at %s:%d"; break;
    case E_OPTION_ERROR       : format = "In Options: %s"; break;
    default: break;
  }
  char* msg = apr_pvsprintf(_hlvm_pool, format, ap);
  apr_file_printf(_hlvm_stderr, "Error: %s\n", msg);
}

void 
hlvm_error(ErrorCodes ec, ...)
{
  va_list ap;
  va_start(ap,ec);
  hlvm_verror(ec,ap);
}

void 
hlvm_fatal(ErrorCodes ec, ...)
{
  va_list ap;
  va_start(ap,ec);
  hlvm_verror(ec,ap);
  exit(ec);
}

void hlvm_panic(const char* msg) {
  exit(254);
}

}
