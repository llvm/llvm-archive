//===-- VMMethod.h - Compiler representation of a Java method ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Method class that represents a
// compile time representation of a Java class method (java.lang.Method).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_VMMETHOD_H
#define LLVM_JAVA_VMMETHOD_H

#include <llvm/Java/ClassFile.h>

namespace llvm {

  class Function;
  class FunctionType;

}

namespace llvm { namespace Java {

  class VMClass;

  class VMMethod {
    const VMClass* parent_;
    const Method* method_;
    Function* function_;

    friend class VMClass;
    // Interface for VMClass.

    // Create statically bound method reference.
    VMMethod(const VMClass* parent, const Method* method);

  public:
    const VMClass* getParent() const { return parent_; }
    Function* getFunction() const { return function_; }

    // FIXME: remove when transition is complete.
    std::string getNameAndDescriptor() const {
      return method_->getName()->str() + method_->getDescriptor()->str();
    }
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_VMMETHOD_H
