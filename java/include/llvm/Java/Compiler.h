//===-- Compiler.h - Java bytecode compiler ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Java bytecode to LLVM bytecode compiler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_COMPILER_H
#define LLVM_JAVA_COMPILER_H

#include <llvm/Module.h>

namespace llvm { namespace Java {

    namespace {
        class CompilerImpl;
    }

    class Compiler {
    public:
        Compiler();
        ~Compiler();

        void compile(Module& m, const ClassFile& cf);

    private:
        // do not implement
        Compiler(const Compiler&);
        const Compiler& operator=(const Compiler&);

    private:
        CompilerImpl* compilerImpl_;
    };

} } // namespace llvm::Java

#endif//LLVM_JAVA_COMPILER_H
