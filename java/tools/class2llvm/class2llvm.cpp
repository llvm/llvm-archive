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
#include <iostream>

using namespace llvm;

static cl::opt<std::string>
InputClass(cl::Positional, cl::desc("<input class>"));

int main(int argc, char* argv[])
{
  sys::PrintStackTraceOnErrorSignal();
  cl::ParseCommandLineOptions(argc, argv,
                              "classfile to llvm utility");

  try {
    std::auto_ptr<Module> module = Java::compile(InputClass);

    PassManager passes;
    passes.add(createVerifierPass());
    passes.add(new WriteBytecodePass(&std::cout));
    passes.run(*module);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
