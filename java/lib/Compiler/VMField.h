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

namespace llvm {

  class GlobalVariable;

}

namespace llvm { namespace Java {

  class VMClass;

  class VMField {
    const VMClass* clazz_;
    const Field* field_;
    union {
      int index;
      GlobalVariable* global;
    } data_;

    friend class VMClass;
    // Interface for VMClass.

    // Create static field reference.
    VMField(const VMClass* clazz, const Field* field);

    // Create member field reference.
    VMField(const VMClass* clazz, const Field* field, int index)
      : clazz_(clazz),
        field_(field) {
      assert(!isStatic() && "This should be a member field!");
      data_.index = index;
    }


  public:
    const std::string& getName() const { return field_->getName()->str(); }
    bool isStatic() const { return field_->isStatic(); }

    const VMClass* getClass() const { return clazz_; }
    int getMemberIndex() const {
      assert(!isStatic() && "Field should not be static!");
      return data_.index;
    }
    GlobalVariable* getGlobal() const {
      assert(isStatic() && "Field should be static!");
      return data_.global;
    }
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_VMFIELD_H
