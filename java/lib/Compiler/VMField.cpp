//===-- VMField.cpp - Compiler representation of a Java field ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the Field class that represents a
// compile time representation of a Java class field (java.lang.Field).
//
//===----------------------------------------------------------------------===//

#include "VMField.h"
#include "Resolver.h"
#include "VMClass.h"
#include <llvm/Constants.h>

using namespace llvm;
using namespace llvm::Java;

VMField::VMField(const VMClass* parent,
                 const VMClass* clazz,
                 const Field* field)
  : parent_(parent),
    class_(clazz),
    field_(field)
{
  assert(isStatic() && "This should be a static field!");

  // A java static field is constant if it is marked final and has a
  // static initializer.
  bool isConstant = field_->isFinal() && field->getConstantValueAttribute();

  llvm::Constant* init = NULL;
  if (ConstantValueAttribute* attr = field_->getConstantValueAttribute()) {
    init = parent_->getConstant(attr->getValueIndex());
    if (init->getType() != class_->getType())
      init = ConstantExpr::getCast(init, class_->getType());
  }
  else
    init = llvm::Constant::getNullValue(class_->getType());

  data_.global = new GlobalVariable(class_->getType(),
                                    isConstant,
                                    GlobalVariable::ExternalLinkage,
                                    init,
                                    parent_->getName() + '/' + getName(),
                                    parent_->getResolver()->getModule());
}
