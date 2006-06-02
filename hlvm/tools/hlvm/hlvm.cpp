//===-- HLVM Main Program ---------------------------------------*- C++ -*-===//
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
/// @file tools/hlvm/hlvm.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/23
/// @since 0.1.0
/// @brief Implements the main program for the HLVM Virtual Machine
//===----------------------------------------------------------------------===//

#include <hlvm/Base/Memory.h>
#include <hlvm/Reader/XML/XMLReader.h>
#include <hlvm/Writer/XML/XMLWriter.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>
#include <fstream>
#include <iostream>

using namespace llvm;
using namespace hlvm;
static cl::opt<std::string>
ProgramToRun(cl::Positional, cl::desc("URI Of Program To Run"));

int main(int argc, char**argv) 
{
  try {
    initialize(argc,argv);
    cl::ParseCommandLineOptions(argc, argv, 
      "hlvm: High Level Virtual Machine\n");

    std::cout << "Unfortunately, the hlvm virtual machine isn't implemented.\n";
  } catch (const std::string& msg) {
    std::cerr << argv[0] << ": " << msg << "\n";
  } catch (...) {
    std::cerr << argv[0] << ": Unexpected unknown exception occurred.\n";
  }
  return 1;
}
