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
    int index_;

    void init();

    friend class VMClass;
    // Interface for VMClass.

    // Create statically bound method reference.
    VMMethod(const VMClass* parent, const Method* method);

    // Create dynamically bound method reference.
    VMMethod(const VMClass* parent, const Method* method, int index);

  public:
    const VMClass* getParent() const { return parent_; }
    const Method* getMethod() const { return method_; }
    Function* getFunction() const { return function_; }
    int getMethodIndex() const { return index_; }

    bool isStaticallyBound() const {
      return isStatic() || isPrivate() || getName()[0] == '<';
    }
    bool isDynamicallyBound() const { return !isStaticallyBound(); }
    bool isAbstract() const { return method_->isAbstract(); }
    bool isNative() const { return method_->isNative(); }
    bool isPrivate() const { return method_->isPrivate(); }
    bool isStatic() const { return method_->isStatic(); }

    // FIXME: remove when transition is complete.
    const std::string& getName() const { return method_->getName()->str(); }
    const std::string& getDescriptor() const {
      return method_->getDescriptor()->str();
    }
    std::string getNameAndDescriptor() const {
      return getName() + getDescriptor();
    }

  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_VMMETHOD_H
