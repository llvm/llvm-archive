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
#include <stack>
#include <vector>

namespace llvm { namespace Java {

    class Compiler {
    public:
        Module* compile(const ClassFile& cf);

    private:
        void compileMethodInit(Function& function,
                               const ClassFile& cf,
                               const CodeAttribute& codeAttr);

        Value* getOrCreateLocal(unsigned index, const Type* type);

        void compileMethod(Module& module,
                           const ClassFile& cf,
                           const Method& method);

    private:
        typedef std::stack<Value*, std::vector<Value*> > OperandStack;
        typedef std::vector<Value*> Locals;
        typedef std::vector<BasicBlock*> BC2BBMap;

        OperandStack opStack_;
        Locals locals_;
        BC2BBMap bc2bbMap_;
    };

} } // namespace llvm::Java

#endif//LLVM_JAVA_COMPILER_H
