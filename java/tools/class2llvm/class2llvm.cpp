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
#include <llvm/Assembly/PrintModulePass.h>
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
                                "classfile to llvm utility");

    try {
        Java::Compiler compiler;

        Module module(InputClass);
        compiler.compile(module, InputClass);

        PassManager passes;
        passes.add(new PrintModulePass(&std::cout));
        passes.run(module);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
