//===-- Resolver.h - Class resolver for Java classes ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of a Java class resolver. This
// object creates Class objects out of loaded ClassFiles.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_RESOLVER_H
#define LLVM_JAVA_RESOLVER_H

#include "Class.h"
#include <llvm/Java/Bytecode.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <map>
#include <string>

namespace llvm { namespace Java {

  class Resolver {
    Module* module_;
    typedef std::map<std::string, Class> ClassMap;
    ClassMap classMap_;
    unsigned nextInterfaceIndex_;
    const Type* objectBaseType_;
    const Type* objectBaseRefType_;

    const Class* getClassForDesc(const std::string& descriptor);

    const Type* getTypeHelper(const std::string&,
                              unsigned& i,
                              bool memberMethod = false) const;

    std::pair<ClassMap::iterator, bool> insertClass(const Class& clazz) {
      return classMap_.insert(std::make_pair(clazz.getName(), clazz));
    }
    ClassMap::iterator insertClass(ClassMap::iterator i, const Class& clazz) {
      return classMap_.insert(i, std::make_pair(clazz.getName(), clazz));
    }

  public:
    static std::string canonicalizeClassName(const std::string& className) {
      if (className[0] == '[')
        return className;
      else
        return 'L' + className + ';';
    }

    Resolver(Module* module);

    const Type* getObjectBaseType() const { return objectBaseType_; }
    const Type* getObjectBaseRefType() const { return objectBaseRefType_; }

    const Type* getType(const std::string& descriptor,
                        bool memberMethod = false) const;
    const Type* getStorageType(const Type* type) const;

    inline bool isTwoSlotType(const Type* type) const {
      return type == Type::LongTy || type == Type::DoubleTy;
    }

    inline bool isOneSlotType(const Type* type) const {
      return !isTwoSlotType(type);
    }

    const Class* getClass(const std::string& className) {
      return getClassForDesc(canonicalizeClassName(className));
    }
    const Class* getClass(const Field& field) {
      return getClassForDesc(field.getDescriptor()->str());
    }
    const Class* getClass(JType type);

    const Class* getArrayClass(const Class* clazz) {
      return getClassForDesc('[' + clazz->getName());
    }

    unsigned getNextInterfaceIndex() { return nextInterfaceIndex_++; }
    Module* getModule() { return module_; }

  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_RESOLVER_H
