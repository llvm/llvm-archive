//===-- class2llvm.cpp - class2llvm utility ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a sample class reader driver. It is used to drive class
// reader tests.
//
//===----------------------------------------------------------------------===//

#include <llvm/Java/ClassFile.h>
#include <llvm/Java/Compiler.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Bytecode/WriteBytecodePass.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/System/Signals.h>

#include <cstddef>
#include <fstream>
#include <iostream>

using namespace llvm;

static cl::opt<std::string>
InputClass(cl::Positional, cl::desc("<input class>"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Output filename"), cl::value_desc("filename"));

int main(int argc, char* argv[])
{
  sys::PrintStackTraceOnErrorSignal();
  cl::ParseCommandLineOptions(argc, argv,
                              "classfile to llvm utility");

  std::ostream* out = &std::cout;
  if (!OutputFilename.empty()) {
    sys::RemoveFileOnSignal(sys::Path(OutputFilename));
    out = new std::ofstream(OutputFilename.c_str());
  }

  try {
    std::auto_ptr<Module> module = Java::compile(InputClass);

    PassManager passes;
    passes.add(createVerifierPass());
    passes.add(new WriteBytecodePass(out));
    passes.run(*module);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    sys::Path(OutputFilename).destroyFile();
    return EXIT_FAILURE;
  }

  if (!OutputFilename.empty())
    delete out;

  return EXIT_SUCCESS;
}
