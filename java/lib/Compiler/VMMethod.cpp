//===-- VMMethod.cpp - Compiler representation of a Java method -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the Method class that represents a
// compile time representation of a Java class method (java.lang.Method).
//
//===----------------------------------------------------------------------===//

#include "VMMethod.h"
#include "Resolver.h"
#include "VMClass.h"
#include <llvm/Function.h>
#include <llvm/DerivedTypes.h>

using namespace llvm;
using namespace llvm::Java;

void VMMethod::init()
{
  const std::string& methodName = method_->getName()->str();
  const std::string& methodDescriptor = method_->getDescriptor()->str();
  Resolver* resolver = parent_->getResolver();
  const FunctionType* functionType = cast<FunctionType>(
    resolver->getType(methodDescriptor, !method_->isStatic()));
  const std::string& className =
    parent_->getClassFile()->getThisClass()->getName()->str();
  const std::string& functionName =
    className + '/' + methodName + methodDescriptor;
  Module* module = resolver->getModule();
  function_ = module->getOrInsertFunction(functionName, functionType);
}

VMMethod::VMMethod(const VMClass* parent, const Method* method)
  : parent_(parent),
    method_(method),
    index_(-1)
{
  assert(isStaticallyBound() && "This should be a statically bound method!");
  init();
}

VMMethod::VMMethod(const VMClass* parent, const Method* method, int index)
  : parent_(parent),
    method_(method),
    index_(index)
{
  assert(isDynamicallyBound() && "This should be a dynamically bound method!");
  init();
}
