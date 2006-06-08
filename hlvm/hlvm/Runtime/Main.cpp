//===-- Runtime Main Implementation -----------------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Main.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/06/04
/// @since 0.1.0
/// @brief Implements the runtime main program.
//===----------------------------------------------------------------------===//

#include <apr-1/apr_getopt.h>
#include <apr-1/apr_file_io.h>

extern "C" {

#include <hlvm/Runtime/Main.h>
#include <hlvm/Runtime/Program.h>
#include <hlvm/Runtime/Memory.h>
#include <hlvm/Runtime/Error.h>
#include <hlvm/Runtime/Internal.h>

apr_getopt_option_t hlvm_options[] = {
  { "help", 'h', 0, "Provides help on using HLVM" },
  { 0, 0, 0, 0 }
};


/// This is the function called from the real main() in hlvm/tools/hlvm.  We 
/// do this because we don't want to expose the "Main" class to the outside
/// world. The interface to the HLVM Runtime is C even though the
/// implementation uses C++.
int hlvm_runtime_main(int argc, char**argv)
{
  int result = 0;
  try {
    // Initialize APR and HLVM
    _hlvm_initialize();

    // Process the options
    apr_getopt_t* options = 0;
    if (APR_SUCCESS != apr_getopt_init(&options, _hlvm_pool, argc, argv))
      hlvm_panic("Can't initialize apr_getopt");
    options->interleave = 0;
    int ch;
    const char* arg;
    apr_status_t stat = apr_getopt_long(options, hlvm_options, &ch, &arg); 
    while (stat != APR_EOF) {
      switch (stat) {
        case APR_SUCCESS:
        {
          switch (ch) {
            case 'h':
              break;
            default:
              break;
          }
        }
        case APR_BADCH:
        {
          hlvm_error(E_BAD_OPTION,0);
          return E_BAD_OPTION;
        }
        case APR_BADARG:
        {
          hlvm_error(E_MISSING_ARGUMENT,0);
          return E_MISSING_ARGUMENT;
        }
        case APR_EOF:
        default:
        {
          hlvm_panic("Unknown response from apr_getopt_long");
          break;
        }
      }
      stat = apr_getopt_long(options, hlvm_options, &ch, &arg); 
    }

    // Find the function that represents the start point.
    if (options->ind == 0) {
      hlvm_error(E_NO_PROGRAM_NAME,0);
      return E_NO_PROGRAM_NAME;
    } else {
      const char* prog_name = argv[options->ind];
      hlvm_program_type func = hlvm_find_program(prog_name);

      // If we got a start function ..
      if (func) {
        // Invoke it.
        return (*func)(options->argc, options->argv);
      } else {
        // Give an error
        hlvm_error(E_PROGRAM_NOT_FOUND,argv[1]);
        return E_PROGRAM_NOT_FOUND;
      }
    }
  } catch (...) {
    hlvm_error(E_UNHANDLED_EXCEPTION,0);
  }
  return result;
}
}
