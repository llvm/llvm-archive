//===-- VMField.h - Compiler representation of a Java field -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Field class that represents a
// compile time representation of a Java class field (java.lang.Field).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_VMFIELD_H
#define LLVM_JAVA_VMFIELD_H

#include <llvm/Java/ClassFile.h>
#include <cassert>

namespace llvm {

  class Constant;
  class GlobalVariable;

}

namespace llvm { namespace Java {

  class VMClass;

  class VMField {
    const VMClass* parent_;
    const VMClass* class_;
    const Field* field_;
    union {
      int index;
      GlobalVariable* global;
    } data_;

    friend class VMClass;
    // Interface for VMClass.

    // Create static field reference.
    VMField(const VMClass* parent, const Field* field);

    // Create member field reference.
    VMField(const VMClass* parent, const Field* field, int index);

  public:
    const std::string& getName() const { return field_->getName()->str(); }
    const std::string& getDescriptor() const {
      return field_->getDescriptor()->str();
    }
    bool isStatic() const { return field_->isStatic(); }

    const VMClass* getParent() const { return parent_; }
    const VMClass* getClass() const { return class_; }
    int getMemberIndex() const {
      assert(!isStatic() && "Field should not be static!");
      return data_.index;
    }
    GlobalVariable* getGlobal() const {
      assert(isStatic() && "Field should be static!");
      return data_.global;
    }

    llvm::Constant* buildFieldDescriptor() const;
    llvm::Constant* buildFieldOffset() const;
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_VMFIELD_H
