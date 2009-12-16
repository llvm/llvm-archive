//===-------------------- JavaRuntimeJIT.cpp ------------------------------===//
//=== ---- Runtime functions called by code compiled by the JIT -----------===//
//
//                            The VMKit project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include "ClasspathReflect.h"
#include "JavaArray.h"
#include "JavaClass.h"
#include "JavaConstantPool.h"
#include "JavaString.h"
#include "JavaThread.h"
#include "JavaTypes.h"
#include "Jnjvm.h"

#include "j3/OpcodeNames.def"

#include <cstdarg>

using namespace j3;

extern "C" void* j3InterfaceLookup(UserClass* caller, uint32 index) {

  void* res = 0;

  BEGIN_NATIVE_EXCEPTION(1)
  
  UserConstantPool* ctpInfo = caller->getConstantPool();
  if (ctpInfo->ctpRes[index]) {
    res = ctpInfo->ctpRes[index];
  } else {
    UserCommonClass* cl = 0;
    const UTF8* utf8 = 0;
    Signdef* sign = 0;
  
    ctpInfo->resolveMethod(index, cl, utf8, sign);
    assert(cl->isClass() && isInterface(cl->access) && "Wrong type of method");
    res = cl->asClass()->lookupInterfaceMethodDontThrow(utf8, sign->keyName);
    
    assert(res && "Can not found method");
    ctpInfo->ctpRes[index] = (void*)res;
  }
  
    
  END_NATIVE_EXCEPTION

  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (JavaMethod*)obj;
  return res;
}

// Throws if the field is not found.
extern "C" void* j3VirtualFieldLookup(UserClass* caller, uint32 index) {
  
  void* res = 0;
  
  BEGIN_NATIVE_EXCEPTION(1)

  UserConstantPool* ctpInfo = caller->getConstantPool();
  if (ctpInfo->ctpRes[index]) {
    res = ctpInfo->ctpRes[index];
  } else {
  
    UserCommonClass* cl = 0;
    const UTF8* utf8 = 0;
    Typedef* sign = 0;
  
    ctpInfo->resolveField(index, cl, utf8, sign);
 
    UserClass* lookup = cl->isArray() ? cl->super : cl->asClass();
    JavaField* field = lookup->lookupField(utf8, sign->keyName, false, true, 0);
  
    ctpInfo->ctpRes[index] = (void*)field->ptrOffset;
  
    res = (void*)field->ptrOffset;
  }

  END_NATIVE_EXCEPTION

  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (void*)obj;
  return res;
}

// Throws if the field or its class is not found.
extern "C" void* j3StaticFieldLookup(UserClass* caller, uint32 index) {
  
  void* res = 0;
  
  BEGIN_NATIVE_EXCEPTION(1)
  
  UserConstantPool* ctpInfo = caller->getConstantPool();
  
  if (ctpInfo->ctpRes[index]) {
    res = ctpInfo->ctpRes[index];
  } else {
  
    UserCommonClass* cl = 0;
    UserClass* fieldCl = 0;
    const UTF8* utf8 = 0;
    Typedef* sign = 0;
  
    ctpInfo->resolveField(index, cl, utf8, sign);
 
    assert(cl->asClass() && "Lookup a field of something not an array");
    JavaField* field = cl->asClass()->lookupField(utf8, sign->keyName, true,
                                                  true, &fieldCl);
    
    fieldCl->initialiseClass(JavaThread::get()->getJVM());
    void* obj = ((UserClass*)fieldCl)->getStaticInstance();
  
    assert(obj && "No static instance in static field lookup");
  
    void* ptr = (void*)((uint64)obj + field->ptrOffset);
    ctpInfo->ctpRes[index] = ptr;
   
    res = ptr;
  }

  END_NATIVE_EXCEPTION

  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (void*)obj;
  return res;
}

#ifndef WITHOUT_VTABLE
// Throws if the method is not found.
extern "C" void* j3VirtualTableLookup(UserClass* caller, uint32 index, ...) {
  
  void* res = 0;
  
  BEGIN_NATIVE_EXCEPTION(1)
    
  UserCommonClass* cl = 0;
  const UTF8* utf8 = 0;
  Signdef* sign = 0;
  
  caller->getConstantPool()->resolveMethod(index, cl, utf8, sign);
  UserClass* lookup = cl->isArray() ? cl->super : cl->asClass();
  JavaMethod* dmeth = lookup->lookupMethodDontThrow(utf8, sign->keyName, false,
                                                    true, 0);
  if (!dmeth) {
    va_list ap;
    va_start(ap, index);
    JavaObject* obj = va_arg(ap, JavaObject*);
    va_end(ap);
    assert((obj->getClass()->isClass() && 
            obj->getClass()->asClass()->isInitializing()) &&
           "Class not ready in a virtual lookup.");
    // Arg, the bytecode is buggy! Perform the lookup on the object class
    // and do not update offset.
    lookup = obj->getClass()->isArray() ? obj->getClass()->super : 
                                       obj->getClass()->asClass();
    dmeth = lookup->lookupMethod(utf8, sign->keyName, false, true, 0);
  } else {
    caller->getConstantPool()->ctpRes[index] = (void*)dmeth->offset;
  }

#if !defined(ISOLATE_SHARING) && !defined(SERVICE)
  assert(dmeth->classDef->isInitializing() && 
         "Class not ready in a virtual lookup.");
#endif

  res = (void*)dmeth->offset;

  END_NATIVE_EXCEPTION

  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (void*)obj;
  return res;
}
#endif

// Throws if the class is not found.
extern "C" void* j3ClassLookup(UserClass* caller, uint32 index) { 
  
  void* res = 0;
  
  BEGIN_NATIVE_EXCEPTION(1)
   
  UserConstantPool* ctpInfo = caller->getConstantPool();
  UserClass* cl = (UserClass*)ctpInfo->loadClass(index);
  // We can not initialize here, because bytecodes such as CHECKCAST
  // or classes used in catch clauses do not trigger class initialization.
  // This is really sad, because we need to insert class initialization checks
  // in the LLVM code.
  assert(cl && "No cl after class lookup");
  res = (void*)cl;

  END_NATIVE_EXCEPTION
  
  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (void*)obj;
  return res;
}

// Calls Java code.
// Throws if initializing the class throws an exception.
extern "C" UserCommonClass* j3RuntimeInitialiseClass(UserClass* cl) {
  BEGIN_NATIVE_EXCEPTION(1)
 
  cl->resolveClass();
  cl->initialiseClass(JavaThread::get()->getJVM());
  
  END_NATIVE_EXCEPTION

  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (UserCommonClass*)obj;
  return cl;
}

// Calls Java code.
extern "C" JavaObject* j3RuntimeDelegatee(UserCommonClass* cl) {
  JavaObject* res = 0;
  llvm_gcroot(res, 0);

  BEGIN_NATIVE_EXCEPTION(1)
  Jnjvm* vm = JavaThread::get()->getJVM();
  res = cl->getClassDelegatee(vm);
  END_NATIVE_EXCEPTION
  
  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return obj;
  return res;
}

// Throws if one of the dimension is negative.
static JavaArray* multiCallNewIntern(UserClassArray* cl, uint32 len,
                                     sint32* dims, Jnjvm* vm) {
  assert(len > 0 && "Negative size given by VMKit");
 
  JavaArray* _res = cl->doNew(dims[0], vm);
  ArrayObject* res = 0;
  llvm_gcroot(_res, 0);
  llvm_gcroot(res, 0);

  if (len > 1) {
    res = (ArrayObject*)_res;
    UserCommonClass* _base = cl->baseClass();
    assert(_base->isArray() && "Base class not an array");
    UserClassArray* base = (UserClassArray*)_base;
    if (dims[0] > 0) {
      for (sint32 i = 0; i < dims[0]; ++i) {
        res->elements[i] = multiCallNewIntern(base, (len - 1),
                                              &dims[1], vm);
      }
    } else {
      for (uint32 i = 1; i < len; ++i) {
        sint32 p = dims[i];
        if (p < 0) JavaThread::get()->getJVM()->negativeArraySizeException(p);
      }
    }
  }
  return _res;
}

// Throws if one of the dimension is negative.
extern "C" JavaArray* j3MultiCallNew(UserClassArray* cl, uint32 len, ...) {
  JavaArray* res = 0;
  llvm_gcroot(res, 0);

  BEGIN_NATIVE_EXCEPTION(1)

  va_list ap;
  va_start(ap, len);
  sint32* dims = (sint32*)alloca(sizeof(sint32) * len);
  for (uint32 i = 0; i < len; ++i){
    dims[i] = va_arg(ap, int);
  }
  Jnjvm* vm = JavaThread::get()->getJVM();
  res = multiCallNewIntern(cl, len, dims, vm);

  END_NATIVE_EXCEPTION

  return res;
}

// Throws if the class can not be resolved.
extern "C" UserClassArray* j3GetArrayClass(UserCommonClass* cl,
                                              UserClassArray** dcl) {
  UserClassArray* res = 0;

  BEGIN_NATIVE_EXCEPTION(1)
  
  JnjvmClassLoader* JCL = cl->classLoader;
  if (cl->asClass()) cl->asClass()->resolveClass();
  const UTF8* arrayName = JCL->constructArrayName(1, cl->getName());
  
  res = JCL->constructArray(arrayName);
  if (dcl) *dcl = res;

  END_NATIVE_EXCEPTION

  // Since the function is marked readnone, LLVM may move it after the
  // exception check. Therefore, we trick LLVM to check the return value of the
  // function.
  JavaObject* obj = JavaThread::get()->pendingException;
  if (obj) return (UserClassArray*)obj;
  return res;
}

// Does not call Java code. Can not yield a GC.
extern "C" void j3EndJNI(uint32** oldLRN, void** oldBuffer) {
  JavaThread* th = JavaThread::get();
  
  // We're going back to Java
  th->endJNI();
  
  // Update the buffer.
  th->currentSjljBuffer = *oldBuffer;
  
  // Update the number of references.
  th->currentAddedReferences = *oldLRN;


}

extern "C" void* j3StartJNI(uint32* localReferencesNumber,
                               uint32** oldLocalReferencesNumber,
                               void* newBuffer, void** oldBuffer,
                               mvm::KnownFrame* Frame) 
  __attribute__((noinline));

// Never throws. Does not call Java code. Can not yied a GC.
extern "C" void* j3StartJNI(uint32* localReferencesNumber,
                               uint32** oldLocalReferencesNumber,
                               void* newBuffer, void** oldBuffer,
                               mvm::KnownFrame* Frame) {
  
  JavaThread* th = JavaThread::get();
 
  *oldBuffer = th->currentSjljBuffer;
  th->currentSjljBuffer = newBuffer;
 
  *oldLocalReferencesNumber = th->currentAddedReferences;
  th->currentAddedReferences = localReferencesNumber;
  th->startKnownFrame(*Frame);

  th->startJNI(1);

  return Frame->currentFP;
}

// Never throws.
extern "C" void j3JavaObjectAquire(JavaObject* obj) {
  BEGIN_NATIVE_EXCEPTION(1)
  
  llvm_gcroot(obj, 0);
  obj->acquire();

  END_NATIVE_EXCEPTION
}

// Never throws.
extern "C" void j3JavaObjectRelease(JavaObject* obj) {
  BEGIN_NATIVE_EXCEPTION(1)
  
  llvm_gcroot(obj, 0);
  obj->release();
  
  END_NATIVE_EXCEPTION
}

// Does not call any Java code. Can not yield a GC.
extern "C" void j3ThrowException(JavaObject* obj) {
  return JavaThread::get()->throwException(obj);
}

// Never throws.
extern "C" void j3OverflowThinLock(JavaObject* obj) {
  obj->overflowThinLock();
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3NullPointerException() {
  
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateNullPointerException();

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif

  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3NegativeArraySizeException(sint32 val) {
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateNegativeArraySizeException();

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif

  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3OutOfMemoryError(sint32 val) {
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateOutOfMemoryError();

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif

  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3StackOverflowError() {
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateStackOverflowError();

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif

  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3ArithmeticException() {
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateArithmeticException();

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif

  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3ClassCastException(JavaObject* obj,
                                               UserCommonClass* cl) {
  JavaObject *exc = 0;
  llvm_gcroot(obj, 0);
  llvm_gcroot(exc, 0);
  
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateClassCastException(obj, cl);

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif

  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3IndexOutOfBoundsException(JavaObject* obj,
                                                      sint32 index) {
  JavaObject *exc = 0;
  llvm_gcroot(obj, 0);
  llvm_gcroot(exc, 0);
  
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateIndexOutOfBoundsException(index);

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif
  
  return exc;
}

// Creates a Java object and then throws it.
extern "C" JavaObject* j3ArrayStoreException(JavaVirtualTable* VT) {
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();

  BEGIN_NATIVE_EXCEPTION(1)
  
  exc = th->getJVM()->CreateArrayStoreException(VT);

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif
  
  return exc;
}

// Create an exception then throws it.
extern "C" void j3ThrowExceptionFromJIT() {
  JavaObject *exc = 0;
  JavaThread *th = JavaThread::get();
  
  BEGIN_NATIVE_EXCEPTION(1)

  JavaMethod* meth = th->getCallingMethodLevel(0);
  exc = th->getJVM()->CreateUnsatisfiedLinkError(meth);

  END_NATIVE_EXCEPTION

#ifdef DWARF_EXCEPTIONS
  th->throwException(exc);
#else
  th->pendingException = exc;
#endif
  
}

extern "C" void* j3StringLookup(UserClass* cl, uint32 index) {
  
  JavaString** str = 0;
  
  BEGIN_NATIVE_EXCEPTION(1)
  
  UserConstantPool* ctpInfo = cl->getConstantPool();
  const UTF8* utf8 = ctpInfo->UTF8At(ctpInfo->ctpDef[index]);
  str = cl->classLoader->UTF8ToStr(utf8);
#if defined(ISOLATE_SHARING) || !defined(ISOLATE)
  ctpInfo->ctpRes[index] = str;
#endif
  
  END_NATIVE_EXCEPTION

  return (void*)str;
}

extern "C" void j3PrintMethodStart(JavaMethod* meth) {
  fprintf(stderr, "[%p] executing %s.%s\n", (void*)mvm::Thread::get(),
          UTF8Buffer(meth->classDef->name).cString(),
          UTF8Buffer(meth->name).cString());
}

extern "C" void j3PrintMethodEnd(JavaMethod* meth) {
  fprintf(stderr, "[%p] return from %s.%s\n", (void*)mvm::Thread::get(),
          UTF8Buffer(meth->classDef->name).cString(),
          UTF8Buffer(meth->name).cString());
}

extern "C" void j3PrintExecution(uint32 opcode, uint32 index,
                                    JavaMethod* meth) {
  fprintf(stderr, "[%p] executing %s.%s %s at %d\n", (void*)mvm::Thread::get(),
         UTF8Buffer(meth->classDef->name).cString(),
         UTF8Buffer(meth->name).cString(),
         OpcodeNames[opcode], index);
}

#ifdef SERVICE

extern "C" void j3ServiceCallStart(Jnjvm* OldService,
                                 Jnjvm* NewService) {
  fprintf(stderr, "I have switched from %d to %d\n", OldService->IsolateID,
          NewService->IsolateID);

  fprintf(stderr, "Now the thread id is %d\n", mvm::Thread::get()->IsolateID);
}

extern "C" void j3ServiceCallStop(Jnjvm* OldService,
                                Jnjvm* NewService) {
  fprintf(stderr, "End service call\n");
}

#endif


#ifdef ISOLATE_SHARING
extern "C" void* j3StaticCtpLookup(UserClass* cl, uint32 index) {
  UserConstantPool* ctpInfo = cl->getConstantPool();
  JavaConstantPool* shared = ctpInfo->getSharedPool();
  uint32 clIndex = shared->getClassIndexFromMethod(index);
  UserClass* refCl = (UserClass*)ctpInfo->loadClass(clIndex);
  refCl->initialiseClass(JavaThread::get()->getJVM());

  CommonClass* baseCl = 0;
  const UTF8* utf8 = 0;
  Signdef* sign = 0;

  shared->resolveMethod(index, baseCl, utf8, sign);
  UserClass* methodCl = 0;
  refCl->lookupMethod(utf8, sign->keyName, true, true, &methodCl);
  ctpInfo->ctpRes[index] = methodCl->getConstantPool();
  shared->ctpRes[clIndex] = refCl->classDef;
  return (void*)methodCl->getConstantPool();
}

extern "C" UserConstantPool* j3SpecialCtpLookup(UserConstantPool* ctpInfo,
                                                   uint32 index,
                                                   UserConstantPool** res) {
  JavaConstantPool* shared = ctpInfo->getSharedPool();
  uint32 clIndex = shared->getClassIndexFromMethod(index);
  UserClass* refCl = (UserClass*)ctpInfo->loadClass(clIndex);

  CommonClass* baseCl = 0;
  const UTF8* utf8 = 0;
  Signdef* sign = 0;

  shared->resolveMethod(index, baseCl, utf8, sign);
  UserClass* methodCl = 0;
  refCl->lookupMethod(utf8, sign->keyName, false, true, &methodCl);
  shared->ctpRes[clIndex] = refCl->classDef;
  *res = methodCl->getConstantPool();
  return methodCl->getConstantPool();
}

#endif
