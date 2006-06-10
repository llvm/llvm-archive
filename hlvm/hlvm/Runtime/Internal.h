//===-- Runtime Internal Sharing Interface ----------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Internal.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/05
/// @since 0.1.0
/// @brief Declares the interface to the internally shared runtime facilities
//===----------------------------------------------------------------------===//

#ifndef HLVM_RUNTIME_INTERNAL_H 
#define HLVM_RUNTIME_INTERNAL_H 

#include <apr-1/apr_file_io.h>
#include <apr-1/apr_strings.h>

extern "C" {

#include <hlvm/Runtime/Error.h>

typedef apr_uint64_t hlvm_size;

struct hlvm_text_obj {
  hlvm_size len;
  char* str;
  unsigned is_const : 1;
};

struct hlvm_stream_obj {
  apr_file_t* fp;
};

struct hlvm_buffer_obj {
  hlvm_size len;
  char* data;
};

extern apr_pool_t* _hlvm_pool;
extern apr_file_t* _hlvm_stderr;

extern void _hlvm_initialize();

/// This is the HLVM runtime assert macro. It is very much similar to
/// the "assert.h" version but without some of the overhead. It also lets
/// us take control of what to do when an assertion happens. The standard
/// implementation just prints and aborts.
#define hlvm_stringize(expr) #expr
#define hlvm_assert(expr) \
  ((expr) ? 1 : hlvm_assert_fail(#expr,__FILE__,__LINE__))


/// This function gets called by the hlvm_assert macro, and in other situations
/// where a "panic" happens. It provides the proper escape mechanism given the
/// configuration of the runtime.
bool hlvm_assert_fail(const char* expression, const char* file, int line_num);

/// This macro checks the return value of an APR call and reports the error
/// if there is one. It can be used as a
#define HLVM_APR(apr_call,whilst) (hlvm_apr_error(apr_call,whilst))

extern bool hlvm_apr_error(apr_status_t stat, const char* whilst);

}

#endif
