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
#include "VMClass.h"
#include <iostream>

using namespace llvm;
using namespace llvm::Java;

VMField::VMField(const VMClass* parent,
                 const VMClass* clazz,
                 const Field* field)
  : parent_(parent),
    clazz_(clazz),
    field_(field)
{
  assert(isStatic() && "This should be a static field!");
  // FIXME: add Global to module.
}
