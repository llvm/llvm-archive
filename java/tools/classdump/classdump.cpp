//===-- classdump.cpp - classdump utility -----------------------*- C++ -*-===//
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
#include <llvm/System/Signals.h>
#include <Support/CommandLine.h>

#include <cstddef>
#include <iostream>

using namespace llvm;

static cl::opt<std::string>
InputClass(cl::Positional, cl::desc("<input class>"));

int main(int argc, char* argv[])
{
  PrintStackTraceOnErrorSignal();
  cl::ParseCommandLineOptions(argc, argv,
                              "class dump utility");

  try {
    const Java::ClassFile* cf(Java::ClassFile::getClassFile(InputClass));

    cf->dump(std::cout);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
