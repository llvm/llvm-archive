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

#define DEBUG_TYPE "javaclass"

#include "VMClass.h"
#include "Resolver.h"
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Support/Debug.h>

using namespace llvm;
using namespace llvm::Java;

// On initialization we create a placeholder global for the class
// record that will be patched later when the class record is
// computed.
void VMClass::init()
{
  classRecord_ = new GlobalVariable(
    OpaqueType::get(),
    false,
    GlobalVariable::ExternalLinkage,
    NULL,
    getName() + "<classRecord>",
    resolver_->getModule());
}

VMClass::VMClass(Resolver* resolver, const std::string& className)
  : name_(className),
    descriptor_(Resolver::canonicalizeClassName(className)),
    resolver_(resolver),
    classFile_(ClassFile::get(className)),
    componentClass_(NULL),
    layoutType_(OpaqueType::get()),
    type_(PointerType::get(layoutType_)),
    interfaceIndex_(INVALID_INTERFACE_INDEX),
    resolvedConstantPool_(classFile_->getNumConstants())
{
  init();
}

VMClass::VMClass(Resolver* resolver, const VMClass* componentClass)
  : name_('[' + componentClass->getDescriptor()),
    descriptor_(name_),
    resolver_(resolver),
    classFile_(NULL),
    componentClass_(componentClass),
    layoutType_(OpaqueType::get()),
    type_(PointerType::get(layoutType_)),
    interfaceIndex_(INVALID_INTERFACE_INDEX)
{
  init();
}

VMClass::VMClass(Resolver* resolver, const Type* type)
  : name_(type == Type::SByteTy  ? "byte" :
          type == Type::UShortTy ? "char" :
          type == Type::DoubleTy ? "double" :
          type == Type::FloatTy  ? "float" :
          type == Type::IntTy    ? "int" :
          type == Type::LongTy   ? "long" :
          type == Type::ShortTy  ? "short" :
          type == Type::BoolTy   ? "boolean" : "void"),
    descriptor_(type == Type::SByteTy  ? "B" :
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
  init();
}

const VMField* VMClass::lookupField(const std::string& name) const
{
  if (const VMField* field = getField(name))
    return field;

  for (unsigned i = 0, e = getNumInterfaces(); i != e; ++i) {
    const VMClass* interface = getInterface(i);
    if (const VMField* field = interface->getField(name))
      return field;
  }

  for (unsigned i = 0, e = getNumSuperClasses(); i != e; ++i) {
    const VMClass* superClass = getSuperClass(i);
    if (const VMField* field = superClass->getField(name))
      return field;
  }

  return NULL;
}

const VMMethod* VMClass::lookupMethod(const std::string& nameAndType) const
{
  if (const VMMethod* method = getMethod(nameAndType))
    return method;

  if (isInterface())
    for (unsigned i = 0, e = getNumInterfaces(); i != e; ++i) {
      const VMClass* interface = getInterface(i);
      if (const VMMethod* method = interface->getMethod(nameAndType))
        return method;
    }
  else
    for (unsigned i = 0, e = getNumSuperClasses(); i != e; ++i) {
      const VMClass* superClass = getSuperClass(i);
      if (const VMMethod* method = superClass->getMethod(nameAndType))
        return method;
    }

  return NULL;
}

void VMClass::computeLayout()
{
  DEBUG(std::cerr << "Computing layout for: " << getName() << '\n');
  // The layout of primitive classes is already computed.
  if (isPrimitive()) {
    DEBUG(std::cerr << "Computed layout for: " << getName() << '\n');
    return;
  }

  std::vector<const Type*> layout;
  if (isArray()) {
    layout.reserve(3);
    layout.push_back(resolver_->getClass("java/lang/Object")->getLayoutType());
    layout.push_back(Type::UIntTy);
    layout.push_back(ArrayType::get(componentClass_->getType(), 0));
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

  DEBUG(std::cerr << "Computed layout for: " << getName() << '\n');
}

llvm::Constant* VMClass::buildSuperClassRecords() const
{
  std::vector<llvm::Constant*> init;
  init.reserve(getNumSuperClasses());
  for (unsigned i = 0, e = getNumSuperClasses(); i != e; ++i)
    init.push_back(ConstantExpr::getCast(
                     getSuperClass(i)->getClassRecord(),
                     resolver_->getClassRecordPtrType()));

  const ArrayType* superClassRecordsType =
    ArrayType::get(resolver_->getClassRecordPtrType(), init.size());

  return ConstantExpr::getPtrPtrFromArrayPtr(
    new GlobalVariable(
      superClassRecordsType,
      true,
      GlobalVariable::ExternalLinkage,
      ConstantArray::get(superClassRecordsType, init),
      getName() + "<superClassRecords>",
      resolver_->getModule()));
}

llvm::Constant*
VMClass::buildInterfaceClassRecord(const VMClass* interface) const
{
  assert(interface->isInterface() && "Must be passed an interface!");

  std::vector<llvm::Constant*> init;
  init.reserve(interface->dynamicallyBoundMethods_.size()+1);
  // Insert a null type info for this interface.
  init.push_back(llvm::Constant::getNullValue(resolver_->getTypeInfoType()));
  // For each method this interface declares, find the corresponding
  // method in this class and put it in its slot.
  for (unsigned i = 0, e = interface->dynamicallyBoundMethods_.size();
       i != e; ++i) {
    assert(init.size() == i+1 && "Interface method not found in class!");
    const VMMethod* interfaceMethod = interface->dynamicallyBoundMethods_[i];
    for (unsigned j = 0, f = dynamicallyBoundMethods_.size(); j != f; ++j) {
      const VMMethod* method = dynamicallyBoundMethods_[j];
      if (method->getName() == interfaceMethod->getName() &&
          method->getDescriptor() == interfaceMethod->getDescriptor()) {
        init.push_back(method->getFunction());
        break;
      }
    }
  }

  llvm::Constant* classRecordInit = ConstantStruct::get(init);

  return ConstantExpr::getCast(
    new GlobalVariable(
      classRecordInit->getType(),
      true,
      GlobalVariable::ExternalLinkage,
      classRecordInit,
      getName() + '+' + interface->getName() + "<classRecord>",
      resolver_->getModule()),
    resolver_->getClassRecordPtrType());
}

llvm::Constant* VMClass::buildInterfaceClassRecords() const
{
  // This is an interface or primitive class record so it doesn't
  // implement any interfaces. Thus the pointer to the array of
  // implemented interfaces is null.
  if (isInterface() || isPrimitive()) {
    const Type* classRecordPtrPtrType =
      PointerType::get(resolver_->getClassRecordPtrType());

    return llvm::Constant::getNullValue(classRecordPtrPtrType);
  }

  // Otherwise this is a class or array class record so we have to
  // fill in the array of implemented interfaces up the max interface
  // index and build each individual interface class record for this
  // class.
  llvm::Constant* nullClassRecord =
    llvm::Constant::getNullValue(resolver_->getClassRecordPtrType());
  std::vector<llvm::Constant*> init(getInterfaceIndex()+1, nullClassRecord);

  for (unsigned i = 0, e = getNumInterfaces(); i != e; ++i) {
    const VMClass* interface = getInterface(i);
    init[interface->getInterfaceIndex()] = buildInterfaceClassRecord(interface);
  }

  const ArrayType* interfaceClassRecordsType =
    ArrayType::get(resolver_->getClassRecordPtrType(), init.size());

  return ConstantExpr::getPtrPtrFromArrayPtr(
    new GlobalVariable(
      interfaceClassRecordsType,
      true,
      GlobalVariable::ExternalLinkage,
      ConstantArray::get(interfaceClassRecordsType, init),
      getName() + "<interfaceClassRecords>",
      resolver_->getModule()));
}

llvm::Constant* VMClass::buildClassName() const
{
  llvm::Constant* name = ConstantArray::get(getName());

  return ConstantExpr::getPtrPtrFromArrayPtr(
    new GlobalVariable(
      name->getType(),
      true,
      GlobalVariable::ExternalLinkage,
      name,
      getName() + "<classname>",
      resolver_->getModule()));
}

llvm::Constant* VMClass::buildFieldDescriptors() const
{
  std::vector<llvm::Constant*> init;
  init.reserve(memberFields_.size()+1);

  for (unsigned i = 0, e = memberFields_.size(); i != e; ++i) {
    const VMField* field = memberFields_[i];
    init.push_back(field->buildFieldDescriptor());
  }
  // Null terminate.
  init.push_back(llvm::Constant::getNullValue(PointerType::get(Type::SByteTy)));

  const ArrayType* arrayType =
    ArrayType::get(init.back()->getType(), init.size());

  return ConstantExpr::getPtrPtrFromArrayPtr(
    new GlobalVariable(
      arrayType,
      true,
      GlobalVariable::ExternalLinkage,
      ConstantArray::get(arrayType, init),
      getName() + "<fielddescriptors>",
      resolver_->getModule()));
}

llvm::Constant* VMClass::buildFieldOffsets() const
{
  std::vector<llvm::Constant*> init;
  init.reserve(memberFields_.size());

  for (unsigned i = 0, e = memberFields_.size(); i != e; ++i) {
    const VMField* field = memberFields_[i];
    init.push_back(field->buildFieldOffset());
  }

  const ArrayType* arrayType = ArrayType::get(Type::UIntTy, init.size());

  return ConstantExpr::getPtrPtrFromArrayPtr(
    new GlobalVariable(
      arrayType,
      true,
      GlobalVariable::ExternalLinkage,
      ConstantArray::get(arrayType, init),
      getName() + "<fieldoffsets>",
      resolver_->getModule()));
}

llvm::Constant* VMClass::buildClassTypeInfo() const
{
  std::vector<llvm::Constant*> init;
  init.reserve(5);

  init.push_back(buildClassName());
  init.push_back(ConstantSInt::get(Type::IntTy, getNumSuperClasses()));
  init.push_back(buildSuperClassRecords());
  init.push_back(ConstantSInt::get(Type::IntTy, getInterfaceIndex()));
  init.push_back(buildInterfaceClassRecords());
  if (isArray())
    init.push_back(ConstantExpr::getCast(getComponentClass()->getClassRecord(),
                                         resolver_->getClassRecordPtrType()));
  else
    init.push_back(
      llvm::Constant::getNullValue(resolver_->getClassRecordPtrType()));
  if (isArray())
    init.push_back(
      ConstantExpr::getCast(
        ConstantExpr::getSizeOf(getComponentClass()->getType()), Type::IntTy));
  else if (isPrimitive())
    init.push_back(ConstantSInt::get(Type::IntTy, -2));
  else if (isInterface())
    init.push_back(ConstantSInt::get(Type::IntTy, -1));
  else // A class.
    init.push_back(ConstantSInt::get(Type::IntTy, 0));

  init.push_back(buildFieldDescriptors());
  init.push_back(buildFieldOffsets());

  return ConstantStruct::get(init);
}

void VMClass::computeClassRecord()
{
  DEBUG(std::cerr << "Computing class record for: " << getName() << '\n');
  // Find dynamically bound methods.
  if (!isPrimitive()) {
    if (const VMClass* superClass = getSuperClass())
      dynamicallyBoundMethods_ = superClass->dynamicallyBoundMethods_;

    if (getClassFile()) {
      const Methods& methods = classFile_->getMethods();
      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
        Method* method = methods[i];
        const std::string& name = method->getName()->str();
        const std::string& descriptor = method->getDescriptor()->str();

        // If method is statically bound just create it.
        if (method->isPrivate() || method->isStatic() || name[0] == '<')
          methodMap_.insert(
            std::make_pair(name + descriptor, VMMethod(this, method)));
        // Otherwise we need to assign an index for it and update the
        // dynamicallyBoundMethods_ vector.
        else {
          const VMMethod* overridenMethod = NULL;
          for (unsigned i = 0, e = getNumDynamicallyBoundMethods();
               i != e; ++i) {
            const VMMethod* m = getDynamicallyBoundMethod(i);
            if (m->getName() == name && m->getDescriptor() == descriptor)
              overridenMethod = m;
          }

          // If this is an overriden method reuse the method index
          // with the overriding one.
          if (overridenMethod) {
            int index = overridenMethod->getMethodIndex();
            MethodMap::iterator i = methodMap_.insert(
              std::make_pair(name + descriptor,
                             VMMethod(this, method, index))).first;
            dynamicallyBoundMethods_[index] = &i->second;
          }
          // Otherwise assign it a new index.
          else {
            int index = dynamicallyBoundMethods_.size();
            MethodMap::iterator i = methodMap_.insert(
              std::make_pair(
                name + descriptor, VMMethod(this, method, index))).first;
            dynamicallyBoundMethods_.push_back(&i->second);
          }
        }
      }
    }
  }

  std::vector<llvm::Constant*> init;
  init.reserve(1 + getNumDynamicallyBoundMethods());
  init.push_back(buildClassTypeInfo());
  for (unsigned i = 0, e = getNumDynamicallyBoundMethods(); i != e; ++i) {
    const VMMethod* method = getDynamicallyBoundMethod(i);
    init.push_back(
      method->isAbstract() ?
      llvm::Constant::getNullValue(method->getFunction()->getType()) :
      method->getFunction());
  }

  llvm::Constant* classRecordInit = ConstantStruct::get(init);
  resolver_->getModule()->addTypeName("classRecord." + getName(),
                                      classRecordInit->getType());

  // Now resolve the opaque type of the placeholder class record.
  const Type* classRecordType =
    cast<PointerType>(classRecord_->getType())->getElementType();
  OpaqueType* opaqueType = cast<OpaqueType>(const_cast<Type*>(classRecordType));
  opaqueType->refineAbstractTypeTo(classRecordInit->getType());
  // Set the initializer of the class record.
  classRecord_->setInitializer(classRecordInit);
  // Mark the class record as constant.
  classRecord_->setConstant(true);

  DEBUG(std::cerr << "Computed class record for: " << getName() << '\n');
}

void VMClass::link()
{
  // Primitive classes require no linking.
  if (isPrimitive())
    ;
  else if (isArray()) {
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

      // The first class is the direct super class of this class.
      superClasses_.reserve(superClass->getNumSuperClasses() + 1);
      superClasses_.push_back(superClass);
      for (unsigned i = 0, e = superClass->getNumSuperClasses(); i != e; ++i)
        superClasses_.push_back(superClass->getSuperClass(i));
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

  // The interface index for an interface is a unique number generated
  // from the resolver.
  if (isInterface())
    interfaceIndex_ = resolver_->getNextInterfaceIndex();
  // For a class it is the max index of all the interfaces it implements.
  else {
    for (unsigned i = 0, e = getNumInterfaces(); i != e; ++i)
      interfaceIndex_ =
        std::max(interfaceIndex_, getInterface(i)->getInterfaceIndex());
  }

  computeLayout();
  computeClassRecord();

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

const VMMethod* VMClass::getMethod(unsigned index) const
{
  assert(classFile_ && "No constant pool!");
  assert((dynamic_cast<ConstantMethodRef*>(classFile_->getConstant(index)) ||
          dynamic_cast<ConstantInterfaceMethodRef*>(classFile_->getConstant(index))) &&
         "Not an index to a method reference!");

  // If we haven't resolved this constant already, do so now.
  if (!resolvedConstantPool_[index]) {
    ConstantMemberRef* jc = classFile_->getConstantMemberRef(index);
    const VMClass* clazz = getClass(jc->getClassIndex());
    ConstantNameAndType* ntc = jc->getNameAndType();
    const std::string& name = ntc->getName()->str();
    const std::string& descriptor = ntc->getDescriptor()->str();
    resolvedConstantPool_[index] =
      const_cast<VMMethod*>(clazz->lookupMethod(name + descriptor));
  }

  return static_cast<const VMMethod*>(resolvedConstantPool_[index]);
}
