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
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/STLExtras.h>

using namespace llvm;
using namespace llvm::Java;

void VMMethod::init()
{
  const std::string& methodName = method_->getName()->str();
  const std::string& methodDescriptor = method_->getDescriptor()->str();
  const std::string& functionName =
    parent_->getName() + '/' + methodName + methodDescriptor;

  Resolver* resolver = parent_->getResolver();
  // FIXME: This type should be taken from the owning class's constant
  // pool (parsed only once per class). This means the
  // Resolver::getType() should be moved in VMClass and its return
  // value should be cached in the constant pool along with the
  // others.
  const FunctionType* functionType = cast<FunctionType>(
    resolver->getType(methodDescriptor, !method_->isStatic()));
  Module* module = resolver->getModule();
  function_ = module->getOrInsertFunction(functionName, functionType);

  std::vector<const Type*> argTypes;
  argTypes.reserve(2);
  argTypes.push_back(resolver->getObjectBaseType());
  argTypes.push_back(PointerType::get(Type::SByteTy));
  const FunctionType* bridgeFunctionType =
    FunctionType::get(functionType->getReturnType(), argTypes, false);
  bridgeFunction_ = module->getOrInsertFunction("bridge_to_" + functionName,
                                                bridgeFunctionType);
  BasicBlock* bb = new BasicBlock("entry", bridgeFunction_);
  std::vector<Value*> params;
  params.reserve(functionType->getNumParams());
  Value* objectArg = bridgeFunction_->arg_begin();
  Value* vaList = next(bridgeFunction_->arg_begin());

  if (!method_->isStatic())
    params.push_back(objectArg);
  for (unsigned i = !method_->isStatic(), e = functionType->getNumParams();
       i != e; ++i) {
    const Type* paramType = functionType->getParamType(i);
    const Type* argType = paramType->getVAArgsPromotedType();
    Value* arg = new VAArgInst(vaList, argType, "tmp", bb);
    if (paramType != argType)
      arg = new CastInst(arg, paramType, "tmp", bb);
    params.push_back(arg);
  }
  if (functionType->getReturnType() == Type::VoidTy) {
    new CallInst(function_, params, "", bb);
    new ReturnInst(bb);
  }
  else {
    Value* result = new CallInst(function_, params, "result", bb);
    new ReturnInst(result, bb);
  }
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

llvm::Constant* VMMethod::buildMethodDescriptor() const
{
  llvm::Constant* fd = ConstantArray::get(getName() + getDescriptor());

  return ConstantExpr::getPtrPtrFromArrayPtr(
    new GlobalVariable(
      fd->getType(),
      true,
      GlobalVariable::ExternalLinkage,
      fd,
      getName() + getDescriptor(),
      parent_->getResolver()->getModule()));
}

llvm::Constant* VMMethod::getBridgeFunction() const
{
  return bridgeFunction_;
}
