#include "runtime.h"
#include <stdlib.h>
#include <string.h>

jint llvm_java_is_primitive_class(struct llvm_java_class_record* cr)
{
  return cr->typeinfo.elementSize == -2;
}

jint llvm_java_is_interface_class(struct llvm_java_class_record* cr)
{
  return cr->typeinfo.elementSize == -1;
}

jint llvm_java_is_array_class(struct llvm_java_class_record* cr)
{
  return cr->typeinfo.elementSize > 0;
}

struct llvm_java_class_record* llvm_java_get_class_record(jobject obj) {
  return obj->classRecord;
}

void llvm_java_set_class_record(jobject obj,
                                struct llvm_java_class_record* cr) {
  obj->classRecord = cr;
}

jboolean llvm_java_is_assignable_from(struct llvm_java_class_record* cr,
                                      struct llvm_java_class_record* from) {
  /* trivial case: class records are the same */
  if (cr == from)
    return JNI_TRUE;

  /* if from is a primitive class then they must be of the same class */
  if (llvm_java_is_primitive_class(from))
    return cr == from;

  /* if from is an interface class then the current class must
   * implement that interface */
  if (llvm_java_is_interface_class(from)) {
    int index = from->typeinfo.interfaceIndex;
    return (cr->typeinfo.interfaceIndex >= index &&
            cr->typeinfo.interfaces[index]);
  }

  /* if from is an array class then the component types of must be
   * assignable from */
  if (llvm_java_is_array_class(from))
    return (cr->typeinfo.component &&
            llvm_java_is_assignable_from(cr->typeinfo.component,
                                         from->typeinfo.component));

  /* otherwise this is a class, check if from is a superclass of this
   * class */
  if (cr->typeinfo.depth > from->typeinfo.depth) {
    int index = cr->typeinfo.depth - from->typeinfo.depth - 1;
    return cr->typeinfo.superclasses[index] == from;
  }

  return JNI_FALSE;
}

jboolean llvm_java_is_instance_of(jobject obj,
                                  struct llvm_java_class_record* cr) {
  /* trivial case: a null object can be cast to any type */
  if (!obj)
    return JNI_TRUE;

  return llvm_java_is_assignable_from(obj->classRecord, cr);
}

jint llvm_java_throw(jobject obj) {
  abort();
}

extern struct llvm_java_class_record* llvm_java_class_records;

struct llvm_java_class_record*
llvm_java_find_class(JNIEnv* env, const char* name) {
  struct llvm_java_class_record** cr = &llvm_java_class_records;
  while (*cr)
    if (strcmp((*cr)->typeinfo.name, name) == 0)
      return *cr;

  return NULL;
}

void Java_java_lang_VMSystem_arraycopy(JNIEnv *env, jobject clazz,
                                       jobject srcObj, jint srcStart,
                                       jobject dstObj, jint dstStart,
                                       jint length) {
  struct llvm_java_bytearray* srcArray = (struct llvm_java_bytearray*) srcObj;
  struct llvm_java_bytearray* dstArray = (struct llvm_java_bytearray*) dstObj;
  unsigned nbytes = length * srcObj->classRecord->typeinfo.elementSize;

  jbyte* src = srcArray->data;
  jbyte* dst = dstArray->data;

  // FIXME: Need to perform a proper type check here.
  if (srcObj->classRecord->typeinfo.elementSize !=
      dstObj->classRecord->typeinfo.elementSize)
    llvm_java_throw(NULL);

  src += srcStart * srcObj->classRecord->typeinfo.elementSize;
  dst += dstStart * dstObj->classRecord->typeinfo.elementSize;

  memmove(dst, src, nbytes);
}

void Java_gnu_classpath_VMSystemProperties_preInit(JNIEnv *env, jobject clazz,
                                                   jobject properties) {

}

typedef void (*ClassInitializerFunction)(void);

extern const ClassInitializerFunction llvm_java_class_initializers;

extern void llvm_java_main(int, char**);

int main(int argc, char** argv) {
  const ClassInitializerFunction* classInit = &llvm_java_class_initializers;
  while (*classInit)
    (*classInit++)();

  llvm_java_main(argc, argv);
  return 0;
}
