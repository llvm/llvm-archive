//===-- Class.h - Compiler representation of a Java class -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Class class that represents a
// compile time representation of a Java class (java.lang.Class).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_CLASS_H
#define LLVM_JAVA_CLASS_H

#include <llvm/Constant.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <map>
#include <vector>

namespace llvm { namespace Java {

  class ClassFile;
  class Resolver;

  class Class {
    static const unsigned INVALID_INTERFACE_INDEX = 0xFFFFFFFF;

    Resolver* resolver_;
    const ClassFile* classFile_;
    const Class* superClass_;
    const Class* componentClass_;
    Type* structType_;
    const Type* type_;
    unsigned interfaceIndex_;
    typedef std::map<std::string, int> Field2IndexMap;
    Field2IndexMap f2iMap_;
    typedef std::vector<const Type*> ElementTypes;
    ElementTypes elementTypes_;
    mutable std::vector<void*> resolvedConstantPool_;

    void addField(const std::string& name, const Type* type);
    void resolveType();

    friend class Resolver;

    // Resolver interface.

    // Load primitive class for type.
    Class(Resolver& resolver, const Type* type);

    // Load class by name.
    Class(Resolver& resolver, const std::string& className);

    // Load array class of component the passed class.
    Class(Resolver& resolver, const Class& componentClass);

    // Link the class.
    void link();
    // Resolve the class.
    void resolve();
    // Initialize the class.
    void initialize();

  public:
    const Type* getStructType() const { return structType_; }
    const Type* getType() const { return type_; }
    const ClassFile* getClassFile() const { return classFile_; }
    const Class* getSuperClass() const { return superClass_; }
    const Class* getComponentClass() const { return componentClass_; }
    bool isArray() const { return componentClass_; }
    bool isPrimitive() const { return !structType_; }
    unsigned getInterfaceIndex() const { return interfaceIndex_; }
    int getFieldIndex(const std::string& name) const;

    llvm::Constant* getConstant(unsigned index) const;
    const Class* getClass(unsigned index) const;
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_CLASS_H
