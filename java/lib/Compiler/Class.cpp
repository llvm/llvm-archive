//===-- Class.cpp - Compiler representation of a Java class -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Class class that represents a
// compile time representation of a Java class (java.lang.Class). This unlike
// a classfile representation, it resolves the constant pool, creates global
// variables for the static members of this class and also creates the class
// record (vtable) of this class.
//
//===----------------------------------------------------------------------===//

#include "Class.h"
#include "Resolver.h"
#include <llvm/DerivedTypes.h>
#include <llvm/Java/ClassFile.h>

#define LLVM_JAVA_OBJECT_BASE "struct.llvm_java_object_base"

using namespace llvm;
using namespace llvm::Java;

Class::Class(Resolver& resolver)
  : resolver_(&resolver),
    classFile_(NULL),
    superClass_(NULL),
    componentClass_(NULL),
    structType_(OpaqueType::get()),
    type_(PointerType::get(structType_)),
    interfaceIndex_(INVALID_INTERFACE_INDEX)
{

}

Class::Class(Resolver& resolver, const Type* type)
  : resolver_(&resolver),
    classFile_(NULL),
    superClass_(NULL),
    componentClass_(NULL),
    structType_(0),
    type_(type),
    interfaceIndex_(INVALID_INTERFACE_INDEX)
{

}

void Class::addField(const std::string& name, const Type* type)
{
  f2iMap_.insert(std::make_pair(name, elementTypes.size()));
  elementTypes.push_back(type);
}

int Class::getFieldIndex(const std::string& name) const {
  Field2IndexMap::const_iterator it = f2iMap_.find(name);
  return it == f2iMap_.end() ? -1 : it->second;
}

void Class::resolveType() {
  PATypeHolder holder = structType_;
  Type* resolvedType = StructType::get(elementTypes);
  cast<OpaqueType>(structType_)->refineAbstractTypeTo(resolvedType);
  structType_ = holder.get();
  type_ = PointerType::get(structType_);
}

void Class::loadClass(const std::string& className)
{
  classFile_ = ClassFile::get(className);

  // This is any class but java/lang/Object.
  if (classFile_->getSuperClass()) {
    const Class& superClass =
      resolver_->getClass(classFile_->getSuperClass()->getName()->str());

    // We first add the struct of the super class.
    addField("super", superClass.getStructType());

    // Although we can safely assume that all interfaces inherits from
    // java/lang/Object, java/lang/Class.getSuperclass() returns null
    // on interface types. So we only set the superClass_ field when
    // the class is not an interface type, but we model the LLVM type
    // of the interface to be as if it inherits java/lang/Object.
    if (classFile_->isInterface())
      interfaceIndex_ = resolver_->getNextInterfaceIndex();
    else
      superClass_ = &superClass;
  }
  // This is java/lang/Object.
  else
    addField("base", resolver_->getObjectBaseType());

  // Then we add the rest of the fields.
  const Fields& fields = classFile_->getFields();
  for (unsigned i = 0, e = fields.size(); i != e; ++i) {
    Field& field = *fields[i];
    if (!field.isStatic())
      addField(field.getName()->str(), resolver_->getClass(field).getType());
  }

  resolveType();

  assert(!isa<OpaqueType>(getStructType()) &&"Class not initialized properly!");
}

void Class::loadArrayClass(const Class& componentClass)
{
  superClass_ = &resolver_->getClass("java/lang/Object");
  componentClass_ = &componentClass;
  addField("super", superClass_->getStructType());
  addField("<length>", Type::UIntTy);
  addField("<data>", ArrayType::get(componentClass_->getType(), 0));

  resolveType();
}
