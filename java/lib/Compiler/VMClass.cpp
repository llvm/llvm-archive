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
    structType_(OpaqueType::get()),
    type_(PointerType::get(structType_)),
    interfaceIndex_(INVALID_INTERFACE_INDEX),
    resolvedConstantPool_(classFile_->getNumConstants())
{

}

VMClass::VMClass(Resolver* resolver, const VMClass* componentClass)
  : name_('[' + componentClass->getName()),
    resolver_(resolver),
    classFile_(NULL),
    componentClass_(componentClass),
    structType_(OpaqueType::get()),
    type_(PointerType::get(structType_)),
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
    structType_(NULL),
    type_(type),
    interfaceIndex_(INVALID_INTERFACE_INDEX)
{

}

void VMClass::addField(const std::string& name, const Type* type)
{
  f2iMap_.insert(std::make_pair(name, elementTypes_.size()));
  elementTypes_.push_back(type);
}

int VMClass::getFieldIndex(const std::string& name) const {
  Field2IndexMap::const_iterator it = f2iMap_.find(name);
  return it == f2iMap_.end() ? -1 : it->second;
}

void VMClass::resolveType() {
  PATypeHolder holder = structType_;
  Type* resolvedType = StructType::get(elementTypes_);
  cast<OpaqueType>(structType_)->refineAbstractTypeTo(resolvedType);
  structType_ = holder.get();
  type_ = PointerType::get(structType_);
}

void VMClass::link()
{
  assert(!isPrimitive() && "Should not link primitive classes!");

  if (isArray()) {
    superClasses_.reserve(1);
    superClasses_.push_back(resolver_->getClass("java/lang/Object"));
    addField("super", superClasses_[0]->getStructType());
    addField("<length>", Type::UIntTy);
    addField("<data>", ArrayType::get(componentClass_->getType(), 0));

    interfaces_.reserve(2);
    interfaces_.push_back(resolver_->getClass("java/lang/Cloneable"));
    interfaces_.push_back(resolver_->getClass("java/io/Serializable"));
  }
  else {
    // This is java/lang/Object.
    if (!classFile_->getSuperClass())
      addField("base", resolver_->getObjectBaseType());
    // This is any class but java/lang/Object.
    else {
      // Our direct super class.
      const VMClass* superClass =
        resolver_->getClass(classFile_->getSuperClass()->getName()->str());

      // Add the interfaces of our direct superclass.
      for (unsigned i = 0, e = superClass->getNumInterfaces(); i != e; ++i)
        interfaces_.push_back(superClass->getInterface(i));

      // For each of the interfaces we implement, load it and add that
      // interface and all the interfaces it inherits from.
      for (unsigned i = 0, e = classFile_->getNumInterfaces(); i != e; ++i) {
        const VMClass* interface =
          getClassForClass(classFile_->getInterfaceIndex(i));
        interfaces_.push_back(interface);
        for (unsigned j = 0, f = interface->getNumInterfaces(); j != f; ++j)
          interfaces_.push_back(interface->getInterface(j));
      }

      // Sort the interfaces array and remove duplicates.
      std::sort(interfaces_.begin(), interfaces_.end());
      interfaces_.erase(std::unique(interfaces_.begin(), interfaces_.end()),
                        interfaces_.end());

      // We first add the struct of the super class.
      addField("super", superClass->getStructType());

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

    // Then we add the rest of the fields.
    const Fields& fields = classFile_->getFields();
    for (unsigned i = 0, e = fields.size(); i != e; ++i) {
      Field* field = fields[i];
      if (!field->isStatic())
        addField(field->getName()->str(),
                 getClassForDescriptor(field->getDescriptorIndex())->getType());
    }
  }

  resolveType();

  assert(!isa<OpaqueType>(getStructType()) &&"Class not initialized properly!");
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
      const Type* stringType = stringClass->getStructType();
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

const VMClass* VMClass::getClassForClass(unsigned index) const
{
  assert(classFile_ && "No constant pool!");
  assert(dynamic_cast<ConstantClass*>(classFile_->getConstant(index)) &&
         "Not an index to a class reference!");

  // If we haven't resolved this constant already, do so now.
  if (!resolvedConstantPool_[index]) {
    ConstantClass* jc = classFile_->getConstantClass(index);
    resolvedConstantPool_[index] =
      const_cast<VMClass*>(resolver_->getClass(jc->getName()->str()));
  }

  return static_cast<const VMClass*>(resolvedConstantPool_[index]);
}

const VMClass* VMClass::getClassForDescriptor(unsigned index) const
{
  assert(classFile_ && "No constant pool!");
  assert(dynamic_cast<ConstantUtf8*>(classFile_->getConstant(index)) &&
         "Not an index to a descriptor reference!");

  // If we haven't resolved this constant already, do so now.
  if (!resolvedConstantPool_[index]) {
    ConstantUtf8* jc = classFile_->getConstantUtf8(index);
    resolvedConstantPool_[index] =
      const_cast<VMClass*>(resolver_->getClassForDesc(jc->str()));
  }

  return static_cast<const VMClass*>(resolvedConstantPool_[index]);
}
