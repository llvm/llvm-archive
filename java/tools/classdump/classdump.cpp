//===-- classdump.cpp - classdump utility -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file a sample class reader driver. It is used to drive class
// reader tests.
//
//===----------------------------------------------------------------------===//

#include <llvm/Java/ClassFile.h>

#include <cstddef>
#include <iostream>
#include <memory>

using namespace llvm;

int main(int argc, char* argv[])
{
    try {
        std::auto_ptr<Java::ClassFile> cf(
            Java::ClassFile::readClassFile(std::cin));

        cf->dump(std::cout);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
