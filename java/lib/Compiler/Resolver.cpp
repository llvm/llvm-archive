//===-- Resolver.cpp - Class resolver for Java classes ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the Java class resolver.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "javaresolver"

#include "Resolver.h"
#include <llvm/Java/ClassFile.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Support/Debug.h>

using namespace llvm;
using namespace llvm::Java;

Resolver::Resolver(Module* module)
  : module_(module),
    nextInterfaceIndex_(0),
    objectBaseLayoutType_(OpaqueType::get()),
    objectBaseType_(PointerType::get(objectBaseLayoutType_)),
    classRecordType_(OpaqueType::get()),
    classRecordPtrType_(PointerType::get(classRecordType_))
{
  module_->addTypeName("struct.llvm_java_object_base",
                       getObjectBaseLayoutType());

  // Compute the class record type. A class record looks like:
  //
  // struct class_record {
  //   struct type_info;
  // };
  //
  // struct type_info {
  //   int depth;
  //   struct class_record** superClasses;
  //   int interfaceIndex;
  //   union {
  //     int interfaceFlag;
  //     struct class_record** interfaces;
  //   };
  //   int elementSize;
  // };

  // Compute the type_info type.
  std::vector<const Type*> elements;
  elements.push_back(Type::IntTy);
  elements.push_back(PointerType::get(PointerType::get(classRecordType_)));
  elements.push_back(Type::IntTy);
  elements.push_back(PointerType::get(PointerType::get(classRecordType_)));
  elements.push_back(PointerType::get(classRecordType_));
  elements.push_back(Type::IntTy);
  typeInfoType_ = StructType::get(elements);

  module_->addTypeName("struct.llvm_java_object_typeinfo", getTypeInfoType());

  // Compute the class_record type.
  PATypeHolder holder = classRecordType_;
  cast<OpaqueType>(const_cast<Type*>(classRecordType_))->refineAbstractTypeTo(
    StructType::get(std::vector<const Type*>(1, getTypeInfoType())));
  classRecordType_ = holder.get();

  module_->addTypeName("struct.llvm_java_object_class_record",
                       getClassRecordType());

  classRecordPtrType_ = PointerType::get(classRecordType_);
}

const Type* Resolver::getType(const std::string& descriptor,
                              bool memberMethod) const
{
  unsigned i = 0;
  return getTypeHelper(descriptor, i, memberMethod);
}

const Type* Resolver::getTypeHelper(const std::string& descr,
                                    unsigned& i,
                                    bool memberMethod) const
{
  assert(i < descr.size());
  switch (descr[i++]) {
  case 'B': return Type::SByteTy;
  case 'C': return Type::UShortTy;
  case 'D': return Type::DoubleTy;
  case 'F': return Type::FloatTy;
  case 'I': return Type::IntTy;
  case 'J': return Type::LongTy;
  case 'S': return Type::ShortTy;
  case 'Z': return Type::BoolTy;
  case 'V': return Type::VoidTy;
  case 'L': {
    unsigned e = descr.find(';', i);
    i = e + 1;
    return getObjectBaseType();
  }
  case '[':
    // Skip '['s.
    if (descr[i] == '[')
      do { ++i; } while (descr[i] == '[');
    // Consume the element type
    getTypeHelper(descr, i);
    return getObjectBaseType();
  case '(': {
    std::vector<const Type*> params;
    if (memberMethod)
      params.push_back(getObjectBaseType());
    while (descr[i] != ')')
      params.push_back(getTypeHelper(descr, i));
    return FunctionType::get(getTypeHelper(descr, ++i), params, false);
  }
    // FIXME: Throw something
  default:  assert(0 && "Cannot parse type descriptor!");
  }
  return 0; // not reached
}

const VMClass* Resolver::getClassForDesc(const std::string& descriptor)
{
  ClassMap::iterator it = classMap_.lower_bound(descriptor);
  if (it == classMap_.end() || it->first != descriptor) {
    DEBUG(std::cerr << "Loading class: " << descriptor << '\n');
    switch (descriptor[0]) {
    case 'B':
      it = insertClass(it, VMClass(this, Type::SByteTy));
      break;
    case 'C':
      it = insertClass(it, VMClass(this, Type::UShortTy));
      break;
    case 'D':
      it = insertClass(it, VMClass(this, Type::DoubleTy));
      break;
    case 'F':
      it = insertClass(it, VMClass(this, Type::FloatTy));
      break;
    case 'I':
      it = insertClass(it, VMClass(this, Type::IntTy));
      break;
    case 'J':
      it = insertClass(it, VMClass(this, Type::LongTy));
      break;
    case 'S':
      it = insertClass(it, VMClass(this, Type::ShortTy));
      break;
    case 'Z':
      it = insertClass(it, VMClass(this, Type::BoolTy));
      break;
    case 'V':
      it = insertClass(it, VMClass(this, Type::VoidTy));
      break;
    case 'L': {
      unsigned pos = descriptor.find(';', 1);
      const std::string& className = descriptor.substr(1, pos - 1);
      it = insertClass(it, VMClass(this, className));
      break;
    }
    case '[': {
      const std::string& componentDescriptor = descriptor.substr(1);
      it = insertClass(it, VMClass(this, getClassForDesc(componentDescriptor)));
      break;
    }
    default:
      assert(0 && "Cannot parse type descriptor!");
      abort();
    }
    it->second.link();
    if (!it->second.isPrimitive() && !it->second.isInterface())
      module_->addTypeName("struct." + descriptor, it->second.getLayoutType());
    DEBUG(std::cerr << "Loaded class: " << descriptor);
    DEBUG(std::cerr << " (" << it->second.getInterfaceIndex() << ")\n");
  }

  return &it->second;
}

const VMClass* Resolver::getClass(JType type)
{
  switch (type) {
  case BOOLEAN: return getClassForDesc("Z");
  case CHAR: return getClassForDesc("C");
  case FLOAT: return getClassForDesc("F");
  case DOUBLE: return getClassForDesc("D");
  case BYTE: return getClassForDesc("B");
  case SHORT: return getClassForDesc("S");
  case INT: return getClassForDesc("I");
  case LONG: return getClassForDesc("J");
  default: assert(0 && "Unhandled JType!"); abort();
  }
}

const Type* Resolver::getStorageType(const Type* type) const
{
  if (isa<PointerType>(type))
    return getObjectBaseType();
  else if (type == Type::BoolTy ||
           type == Type::UByteTy ||
           type == Type::SByteTy ||
           type == Type::UShortTy ||
           type == Type::ShortTy ||
           type == Type::UIntTy)
    return Type::IntTy;
  else if (type == Type::ULongTy)
    return Type::LongTy;
  else
    return type;
}
