//===-- VMClass.cpp - Compiler representation of a Java class ---*- C++ -*-===//
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

#include "VMClass.h"
#include "Resolver.h"
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Java/ClassFile.h>

#define LLVM_JAVA_OBJECT_BASE "struct.llvm_java_object_base"

using namespace llvm;
using namespace llvm::Java;

VMClass::VMClass(Resolver* resolver, const std::string& className)
  : name_(Resolver::canonicalizeClassName(className)),
    resolver_(resolver),
    classFile_(ClassFile::get(className)),
    componentClass_(NULL),
    layoutType_(OpaqueType::get()),
    type_(PointerType::get(layoutType_)),
    interfaceIndex_(INVALID_INTERFACE_INDEX),
    resolvedConstantPool_(classFile_->getNumConstants())
{

}

VMClass::VMClass(Resolver* resolver, const VMClass* componentClass)
  : name_('[' + componentClass->getName()),
    resolver_(resolver),
    classFile_(NULL),
    componentClass_(componentClass),
    layoutType_(OpaqueType::get()),
    type_(PointerType::get(layoutType_)),
    interfaceIndex_(INVALID_INTERFACE_INDEX)
{

}

VMClass::VMClass(Resolver* resolver, const Type* type)
  : name_(type == Type::SByteTy  ? "B" :
          type == Type::UShortTy ? "C" :
          type == Type::DoubleTy ? "D" :
          type == Type::FloatTy  ? "F" :
          type == Type::IntTy    ? "I" :
          type == Type::LongTy   ? "J" :
          type == Type::ShortTy  ? "S" :
          type == Type::BoolTy   ? "Z" : "V"),
    resolver_(resolver),
    classFile_(NULL),
    componentClass_(NULL),
    layoutType_(const_cast<Type*>(type)),
    type_(type),
    interfaceIndex_(INVALID_INTERFACE_INDEX)
{

}

const VMField* VMClass::lookupField(const std::string& name) const
{
  FieldMap::const_iterator it = fieldMap_.find(name);
  if (it != fieldMap_.end())
    return &it->second;

  for (unsigned i = 0, e = getNumInterfaces(); i != e; ++i) {
    const VMClass* interface = getInterface(i);
    it = interface->fieldMap_.find(name);
    if (it != interface->fieldMap_.end())
      return &it->second;
  }

  for (unsigned i = 0, e = getNumSuperClasses(); i != e; ++i) {
    const VMClass* superClass = getSuperClass(i);
    it = superClass->fieldMap_.find(name);
    if (it != superClass->fieldMap_.end())
      return &it->second;
  }

  return NULL;
}

void VMClass::computeLayout()
{
  std::vector<const Type*> layout;
  if (isArray()) {
    layout.reserve(3);
    layout.push_back(resolver_->getClass("java/lang/Object")->getLayoutType());
    layout.push_back(Type::UIntTy);
    layout.push_back(ArrayType::get(componentClass_->getType(), 0));
  }
  else if (isInterface()) {
    layout.reserve(1);
    layout.push_back(resolver_->getClass("java/lang/Object")->getLayoutType());
  }
  else {
    if (const VMClass* superClass = getSuperClass())
      layout.push_back(superClass->getLayoutType());
    else // This is java/lang/Object
      layout.push_back(resolver_->getObjectBaseLayoutType());
    // Now add the fields.
    const Fields& fields = classFile_->getFields();
    for (unsigned i = 0, e = fields.size(); i != e; ++i) {
      Field* field = fields[i];
      const std::string& name = field->getName()->str();
      if (field->isStatic()) {
        fieldMap_.insert(std::make_pair(name, VMField(this, field)));
      }
      else {
        unsigned index = memberFields_.size() + 1;
        FieldMap::iterator i = fieldMap_.insert(
          std::make_pair(name, VMField(this, field, index))).first;
        const VMField* vmf = &i->second;
        memberFields_.push_back(vmf);
        layout.push_back(vmf->getClass()->getType());
      }
    }
  }

  PATypeHolder holder = layoutType_;
  Type* resolvedType = StructType::get(layout);
  cast<OpaqueType>(layoutType_)->refineAbstractTypeTo(resolvedType);
  layoutType_ = holder.get();
  type_ = PointerType::get(layoutType_);
}

void VMClass::link()
{
  // Primitive classes require no linking.
  if (isPrimitive())
    return;

  if (isArray()) {
    superClasses_.reserve(1);
    superClasses_.push_back(resolver_->getClass("java/lang/Object"));

    interfaces_.reserve(2);
    interfaces_.push_back(resolver_->getClass("java/lang/Cloneable"));
    interfaces_.push_back(resolver_->getClass("java/io/Serializable"));
  }
  else {
    // This is any class but java/lang/Object.
    if (classFile_->getSuperClass()) {
      // Our direct super class.
      const VMClass* superClass = getClass(classFile_->getSuperClassIndex());

      // Add the interfaces of our direct superclass.
      for (unsigned i = 0, e = superClass->getNumInterfaces(); i != e; ++i)
        interfaces_.push_back(superClass->getInterface(i));

      // Although we can safely assume that all interfaces inherit
      // from java/lang/Object, java/lang/Class.getSuperclass()
      // returns null on interface types. So we only set the
      // superClass_ field when the class is not an interface type,
      // but we model the LLVM type of the interface to be as if it
      // inherits java/lang/Object.
      if (classFile_->isInterface())
        interfaceIndex_ = resolver_->getNextInterfaceIndex();
      else {
        // Build the super classes array. The first class is the
        // direct super class of this class.
        superClasses_.reserve(superClass->getNumSuperClasses() + 1);
        superClasses_.push_back(superClass);
        for (unsigned i = 0, e = superClass->getNumSuperClasses(); i != e; ++i)
          superClasses_.push_back(superClass->getSuperClass(i));
      }
    }

    // For each of the interfaces we implement, load it and add that
    // interface and all the interfaces it inherits from.
    for (unsigned i = 0, e = classFile_->getNumInterfaces(); i != e; ++i) {
      const VMClass* interface = getClass(classFile_->getInterfaceIndex(i));
      interfaces_.push_back(interface);
      for (unsigned j = 0, f = interface->getNumInterfaces(); j != f; ++j)
        interfaces_.push_back(interface->getInterface(j));
    }

    // Sort the interfaces array and remove duplicates.
    std::sort(interfaces_.begin(), interfaces_.end());
    interfaces_.erase(std::unique(interfaces_.begin(), interfaces_.end()),
                      interfaces_.end());
  }

  computeLayout();

  assert(!isa<OpaqueType>(getLayoutType()) &&"Class not initialized properly!");
}

llvm::Constant* VMClass::getConstant(unsigned index) const
{
  assert(classFile_ && "No constant pool!");
  assert((dynamic_cast<ConstantString*>(classFile_->getConstant(index)) ||
          dynamic_cast<ConstantInteger*>(classFile_->getConstant(index)) ||
          dynamic_cast<ConstantFloat*>(classFile_->getConstant(index)) ||
          dynamic_cast<ConstantLong*>(classFile_->getConstant(index)) ||
          dynamic_cast<ConstantDouble*>(classFile_->getConstant(index))) &&
         "Not an index to a constant!");

  // If we haven't resolved this constant already, do so now.
  if (!resolvedConstantPool_[index]) {
    Constant* jc = classFile_->getConstant(index);
    if (ConstantString* s = dynamic_cast<ConstantString*>(jc)) {
      const VMClass* stringClass = resolver_->getClass("java/lang/String");
      const Type* stringType = stringClass->getLayoutType();
      resolvedConstantPool_[index] =
        new GlobalVariable(stringType,
                           false,
                           GlobalVariable::LinkOnceLinkage,
                           llvm::Constant::getNullValue(stringType),
                           s->getValue()->str() + ".java/lang/String",
                           resolver_->getModule());
    }
    else if (ConstantInteger* i = dynamic_cast<ConstantInteger*>(jc))
      resolvedConstantPool_[index] =
        ConstantSInt::get(Type::IntTy, i->getValue());
    else if (ConstantFloat* f = dynamic_cast<ConstantFloat*>(jc))
      resolvedConstantPool_[index] =
        ConstantFP::get(Type::FloatTy, f->getValue());
    else if (ConstantLong* l = dynamic_cast<ConstantLong*>(jc))
      resolvedConstantPool_[index] =
        ConstantSInt::get(Type::LongTy, l->getValue());
    else if (ConstantDouble* d = dynamic_cast<ConstantDouble*>(jc))
      resolvedConstantPool_[index] =
        ConstantFP::get(Type::DoubleTy, d->getValue());
    else
      assert(0 && "Not a constant!");
  }

  return static_cast<llvm::Constant*>(resolvedConstantPool_[index]);
}

const VMClass* VMClass::getClass(unsigned index) const
{
  assert(classFile_ && "No constant pool!");
  assert((dynamic_cast<ConstantClass*>(classFile_->getConstant(index)) ||
          dynamic_cast<ConstantUtf8*>(classFile_->getConstant(index))) &&
         "Not an index to a class or descriptor reference!");

  // If we haven't resolved this constant already, do so now.
  if (!resolvedConstantPool_[index]) {
    Constant* jc = classFile_->getConstant(index);
    if (ConstantClass* c = dynamic_cast<ConstantClass*>(jc))
      resolvedConstantPool_[index] =
        const_cast<VMClass*>(resolver_->getClass(c->getName()->str()));
    else if (ConstantUtf8* d = dynamic_cast<ConstantUtf8*>(jc))
      resolvedConstantPool_[index] =
        const_cast<VMClass*>(resolver_->getClassForDesc(d->str()));
    else
      assert(0 && "Not a class!");
  }

  return static_cast<const VMClass*>(resolvedConstantPool_[index]);
}

const VMField* VMClass::getField(unsigned index) const
{
  assert(classFile_ && "No constant pool!");
  assert(dynamic_cast<ConstantFieldRef*>(classFile_->getConstant(index)) &&
         "Not an index to a field reference!");

  // If we haven't resolved this constant already, do so now.
  if (!resolvedConstantPool_[index]) {
    ConstantFieldRef* jc = classFile_->getConstantFieldRef(index);
    const VMClass* clazz = getClass(jc->getClassIndex());
    const std::string& name = jc->getNameAndType()->getName()->str();
    resolvedConstantPool_[index] =
      const_cast<VMField*>(clazz->lookupField(name));
  }

  return static_cast<const VMField*>(resolvedConstantPool_[index]);
}
