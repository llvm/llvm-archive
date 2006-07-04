//===-- hlvm-gentestcase main program ---------------------------*- C++ -*-===//
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
/// @file tools/hlvm-gentestcase/hlvm-gentestcase.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Implements the main program for the hlvm-gentestcase executable
//===----------------------------------------------------------------------===//

#include <hlvm/Base/Memory.h>
#include <hlvm/Writer/XMLWriter.h>
#include <hlvm/Pass/Pass.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>
#include <fstream>
#include <iostream>

using namespace llvm;
using namespace hlvm;
static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
               cl::value_desc("filename"));

static cl::opt<std::string>
URL("url", cl::desc("Specify URL for compilation unit"),
    cl::value_desc("url"));

static cl::opt<std::string>
BundleName("bundle", cl::desc("Specify bundle name"),
    cl::value_desc("name"));

static cl::opt<bool>
NoValidate("no-validate",cl::desc("Disable validation of generated code"),
    cl::init(false));

extern AST* GenerateTestCase(const std::string& id, const std::string& bname);

int main(int argc, char**argv) 
{
  try {
    initialize(argc,argv);
    cl::ParseCommandLineOptions(argc, argv, 
      "hlvm-gentestcase - HLVM test case generator\n");

    std::ostream *Out = &std::cout;  // Default to printing to stdout.

    if (BundleName.empty())
      BundleName = "generatedTestCase";

    if (URL.empty())
      URL = std::string("http://hlvm.org/src/test/generated/") + 
              BundleName + ".hlx";

    if (OutputFilename.empty())
      OutputFilename = BundleName + ".hlx";

    if (OutputFilename != "-") { // Not stdout?
      Out = new std::ofstream(OutputFilename.c_str());
      sys::RemoveFileOnSignal(sys::Path(OutputFilename));
    }

    if (!Out->good()) {
      std::cerr << argv[0] << ": error opening " << OutputFilename
                << ": sending to stdout instead!\n";
      Out = &std::cout;
    }

    AST* tree = GenerateTestCase(URL,BundleName);
    if (!NoValidate) {
      if (!validate(tree)) {
        std::cerr << argv[0] << ": Generated test case did not validate.\n";
      }
    }
    XMLWriter* wrtr = XMLWriter::create(OutputFilename.c_str());
    wrtr->write(tree);
    delete wrtr;

    if (Out != &std::cout) {
      static_cast<std::ofstream*>(Out)->close();
      delete Out;
    }
    return 0;
  } catch (const std::string& msg) {
    std::cerr << argv[0] << ": " << msg << "\n";
  } catch (...) {
    std::cerr << argv[0] << ": Unexpected unknown exception occurred.\n";
  }
  return 1;
}
