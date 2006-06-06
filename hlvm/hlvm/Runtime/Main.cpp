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

using namespace llvm;

namespace {

/// Option to indicate which program to start running
static cl::opt<std::string> Start("start",
  cl::Required,
  cl::desc("Specify the starting point for the program"),
  cl::value_desc("program URI")
);

/// This is the Main class that handles the basic execution framework for
/// all HLVM programs.
class Main {
  int argc;
  char ** argv;
public:
  /// Construct the "Main" and handle argument processing.
  Main(int ac, char**av) : argc(ac), argv(av) {
    llvm::cl::SetVersionPrinter(hlvm::print_version);
    llvm::cl::ParseCommandLineOptions(argc,argv,"High Level Virtual Machine\n");
  }

  /// Run the requested program.
  int run() {
    // First, find the function that represents the start point.
    hlvm_program_type func = hlvm_find_program(Start.c_str());

    // If we got a start function ..
    if (func) {
      // Invoke it.
      return (*func)(argc-1,(signed char**)&argv[1]);
    } else {
      // Give an error
      std::cerr << argv[0] << ": Program '" << Start << "' not found.\n";
      return 1;
    }
  }
};

}

extern "C" {

/// This is the function called from the real main() in hlvm/tools/hlvm.  We 
/// do this because we don't want to expose the "Main" class to the outside
/// world. The interface to the HLVM Runtime is C even though the
/// implementation uses C++.
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
