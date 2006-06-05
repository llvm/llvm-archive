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

#include <hlvm/Base/Memory.h>
#include <hlvm/Runtime/Main.h>
#include <hlvm/Runtime/Program.h>
#include <hlvm/Runtime/Memory.h>
#include <llvm/Support/CommandLine.h>
#include <string.h>
#include <iostream>

namespace {

using namespace llvm;

static cl::opt<std::string>
ProgramToRun(cl::Positional, cl::desc("URI of program to run"));

class Main {
  int argc;
  char ** argv;
public:
  Main(int ac, char**av) : argc(ac), argv(av) {
    llvm::cl::ParseCommandLineOptions(argc,argv,"High Level Virtual Machine\n");
  }
  int run() {
    hlvm_program_type func = hlvm_find_program(ProgramToRun.c_str());
    if (func) {
      hlvm_program_args args;
      args.argc = argc - 1;
      args.argv = (hlvm_string*) 
        hlvm_allocate_array(argc-1, sizeof(hlvm_string));
      for (unsigned i = 0; i < args.argc; i++) {
        uint64_t len = strlen(argv[i]);
        args.argv[i].len = len;
        args.argv[i].str = (const char*)
          hlvm_allocate_array(len, sizeof(args.argv[i].str[0]));
      }
      return (*func)(&args);
    } else {
      std::cerr << argv[0] << ": Program '" << ProgramToRun << "' not found.\n";
      return 1;
    }
  }
};

}

extern "C" {

int hlvm_runtime_main(int argc, char**argv)
{
  int result = 0;
  try {
    hlvm::initialize(argc,argv);
    Main main(argc,argv);
    result = main.run();
  } catch (const std::string& msg) {
    std::cerr << argv[0] << ": " << msg;
  } catch (...) {
    std::cerr << argv[0] << ": Unhandled exception\n";
  }
  return result;
}

}
