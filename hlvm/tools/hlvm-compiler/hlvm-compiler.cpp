//===-- HLVM Compiler -------------------------------------------*- C++ -*-===//
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
/// @brief Implements the main program for the HLVM Compiler 
//===----------------------------------------------------------------------===//

#include <hlvm/Base/Memory.h>
#include <hlvm/Reader/XMLReader.h>
#include <hlvm/Writer/XMLWriter.h>
#include <hlvm/Pass/Pass.h>
#include <hlvm/CodeGen/LLVMGenerator.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>
#include <fstream>
#include <iostream>

using namespace llvm;
using namespace hlvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input XML>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
               cl::value_desc("filename"));

static cl::opt<bool> NoValidate("no-validate", cl::init(false),
  cl::desc("Don't validate input before generating code"));

static cl::opt<bool> NoVerify("no-verify", cl::init(false),
  cl::desc("Don't verify generated LLVM module"));

enum GenerationOptions {
  GenLLVMBytecode,
  GenLLVMAssembly,
  GenHLVMXML,
  GenNativeExecutable,
  GenLoadableModule
};

static cl::opt<GenerationOptions> WhatToGenerate(
  cl::Optional,
  cl::desc("Choose what kind of output to generate"),
  cl::init(GenLLVMBytecode),
  cl::values(
    clEnumValN(GenLLVMBytecode,     "llvmbc",  "Generate LLVM Bytecode"),
    clEnumValN(GenLLVMAssembly,     "llvmasm", "Generate LLVM Assembly"),
    clEnumValN(GenNativeExecutable, "native",  "Generate Native Executable"),
    clEnumValN(GenLoadableModule,   "module",  "Generate Loadable Module"),
    clEnumValEnd
  )
);

int main(int argc, char**argv) 
{
  try {
    initialize(argc,argv);
    cl::ParseCommandLineOptions(argc, argv, 
      "hlvm-compile: Compile HLVM AST To Other Forms\n");

    std::ostream *Out = &std::cout;  // Default to printing to stdout.

    if (OutputFilename != "") {   // Specified an output filename?
      if (OutputFilename != "-") { // Not stdout?
        Out = new std::ofstream(OutputFilename.c_str());
        sys::RemoveFileOnSignal(sys::Path(OutputFilename));
      }
    } else {
      if (InputFilename == "-") {
        OutputFilename = "-";
      } else {
        std::string IFN = InputFilename;
        OutputFilename = InputFilename + ".out";

        Out = new std::ofstream(OutputFilename.c_str());
        sys::RemoveFileOnSignal(sys::Path(OutputFilename));
      }
    }

    if (!Out->good()) {
      std::cerr << argv[0] << ": error opening " << OutputFilename
                << ": sending to stdout instead!\n";
      Out = &std::cout;
    }

    if (InputFilename == "-" ) {
      std::cerr << "Not supported yet: input from stdin\n";
      exit(2);
    } else {
      llvm::sys::Path path(InputFilename);
      if (!path.canRead()) {
        std::cerr << argv[0] << ": can't read input file: " << InputFilename
                  << "\n";
        exit(2);
      }
    }

    XMLReader* rdr = XMLReader::create(InputFilename);
    rdr->read();
    AST* node = rdr->get();
    if (node && !NoValidate) {
      if (!validate(node))
        exit(3);
    }
    switch (WhatToGenerate) {
      case GenLLVMBytecode:
        if (!generateBytecode(node,*Out, !NoVerify))
          exit(4);
        break;
      case GenLLVMAssembly: 
        if (!generateAssembly(node,*Out, !NoVerify))
          exit(4);
        break;
      case GenHLVMXML:
      {
        XMLWriter* wrtr = XMLWriter::create(OutputFilename.c_str());
        wrtr->write(node);
        delete wrtr;
        break;
      }
      case GenNativeExecutable:
        std::cerr << argv[0] << 
          ": Generating native executables is not supported yet\n";
        exit(4);
      case GenLoadableModule:
        std::cerr << argv[0] << 
          ": Generating native executables is not supported yet\n";
        exit(4);
      default:
        std::cerr << argv[0] << 
          ": Unknown generation request.\n";
        exit(4);
    }
    delete rdr;

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
