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
#include <fstream>
#include <iostream>
#include <memory>

using namespace llvm;

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bytecode>"), cl::init("-"));

namespace {

    std::auto_ptr<std::istream> getInputStream(const std::string& fn) {
        std::auto_ptr<std::istream> in;
        if (fn == "-") in.reset(new std::istream(std::cin.rdbuf()));
        else in.reset(new std::ifstream(fn.c_str()));

        return in;
    }

}

int main(int argc, char* argv[])
{
    PrintStackTraceOnErrorSignal();
    cl::ParseCommandLineOptions(argc, argv,
                                "classfile to llvm utility");

    try {
        std::auto_ptr<std::istream> in(getInputStream(InputFilename));
        
        std::auto_ptr<Java::ClassFile> cf(Java::ClassFile::readClassFile(*in));

        Java::Compiler compiler;

        Module module(InputFilename);
        compiler.compile(module, *cf);

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
