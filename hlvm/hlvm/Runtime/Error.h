//===-- HLVM Runtime Error Handling Interface -------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Error.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/05
/// @since 0.1.0
/// @brief Declares the interface to the runtime error handler 
//===----------------------------------------------------------------------===//

#ifndef HLVM_RUNTIME_ERROR_H
#define HLVM_RUNTIME_ERROR_H

enum ErrorCodes {
  E_UNHANDLED_EXCEPTION,
  E_BAD_OPTION,
  E_MISSING_ARGUMENT,
  E_NO_PROGRAM_NAME,
  E_PROGRAM_NOT_FOUND,
  E_APR_ERROR,
  E_ASSERT_FAIL,
  E_OPTION_ERROR
};

void hlvm_error(ErrorCodes ec, ...);
void hlvm_verror(ErrorCodes ec, va_list ap);
void hlvm_fatal(ErrorCodes ec, ...);
void hlvm_panic(const char *msg);

#endif
