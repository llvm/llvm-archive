#include <stdlib.h>
#include <string.h>
#include <llvm/Java/jni.h>

struct llvm_java_object_base;
struct llvm_java_object_header;
struct llvm_java_object_vtable;
struct llvm_java_object_typeinfo;

struct llvm_java_object_header {
  /* gc info, hash info, locking */
    int dummy;
};

struct llvm_java_object_base {
  struct llvm_java_object_header header;
  struct llvm_java_object_class_record* classRecord;
};

struct llvm_java_object_typeinfo {
  /* The number of super classes to java.lang.Object. */
  jint depth;

  /* The super class records up to java.lang.Object. */
  struct llvm_java_object_class_record** superclasses;

  /* If an interface its interface index, otherwise the last interface
   * index implemented by this class. */
  jint interfaceIndex;


  /* The interface class records this class implements. */
  struct llvm_java_object_class_record** interfaces;

  /* The component class record if this is an array class, null
   * otherwise. */
  struct llvm_java_object_class_record* component;

  /* If an array the size of its elements, otherwise 0 for classes, -1
   * for interfaces and -2 for primitive classes. */
  jint elementSize;
};

struct llvm_java_object_class_record {
  struct llvm_java_object_typeinfo typeinfo;
};

jint llvm_java_is_primitive_class(struct llvm_java_object_class_record* cr)
{
  return cr->typeinfo.elementSize == -2;
}

jint llvm_java_is_interface_class(struct llvm_java_object_class_record* cr)
{
  return cr->typeinfo.elementSize == -1;
}

jint llvm_java_is_array_class(struct llvm_java_object_class_record* cr)
{
  return cr->typeinfo.elementSize > 0;
}

struct llvm_java_object_class_record* llvm_java_get_class_record(jobject obj) {
  return obj->classRecord;
}

void llvm_java_set_class_record(jobject obj,
                                struct llvm_java_object_class_record* cr) {
  obj->classRecord = cr;
}

jint llvm_java_is_assignable_from(struct llvm_java_object_class_record* cr,
                                  struct llvm_java_object_class_record* from) {
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

jint llvm_java_is_instance_of(jobject obj,
                              struct llvm_java_object_class_record* cr) {
  /* trivial case: a null object can be cast to any type */
  if (!obj)
    return JNI_TRUE;

  return llvm_java_is_assignable_from(obj->classRecord, cr);
}

jint llvm_java_throw(jobject obj) {
  abort();
}

/* The implementation of JNI functions */

#define HANDLE_TYPE(TYPE) \
  struct llvm_java_##TYPE##array { \
    struct llvm_java_object_base object_base; \
    jint length; \
    j##TYPE data[0]; \
  };
#include "types.def"

static jint llvm_java_get_array_length(JNIEnv* env, jarray array) {
  return ((struct llvm_java_booleanarray*) array)->length;
}

#define HANDLE_TYPE(TYPE) \
  static j ## TYPE* llvm_java_get_##TYPE##_array_elements( \
    JNIEnv* env, \
    jarray array, \
    jboolean* isCopy) { \
    if (isCopy) \
      *isCopy = JNI_FALSE; \
    return ((struct llvm_java_ ##TYPE## array*) array)->data; \
  }
#include "types.def"

#define HANDLE_TYPE(TYPE) \
  static void llvm_java_release_ ##TYPE## _array_elements( \
    JNIEnv* env, \
    jarray array, \
    j##TYPE* elements, \
    jint mode) { \
    switch (mode) { \
    case 0: \
    case JNI_COMMIT: \
    case JNI_ABORT: \
      return; \
    default: \
      abort(); \
    } \
  }
#include "types.def"

/* The JNI interface definition */
static const struct JNINativeInterface llvm_java_JNINativeInterface = {
  NULL, /* 0 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 10 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 20 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 30 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 40 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 50 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 60 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 70 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 80 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 90 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 100 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 110 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 120 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 130 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 140 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 150 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 160 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 170 */
  &llvm_java_get_array_length,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 180 */
  NULL,
  NULL,
  &llvm_java_get_boolean_array_elements,
  &llvm_java_get_byte_array_elements,
  &llvm_java_get_char_array_elements,
  &llvm_java_get_short_array_elements,
  &llvm_java_get_int_array_elements,
  &llvm_java_get_long_array_elements,
  &llvm_java_get_float_array_elements,
  &llvm_java_get_double_array_elements,
  &llvm_java_release_boolean_array_elements,
  &llvm_java_release_byte_array_elements,
  &llvm_java_release_char_array_elements,
  &llvm_java_release_short_array_elements,
  &llvm_java_release_int_array_elements,
  &llvm_java_release_long_array_elements,
  &llvm_java_release_float_array_elements,
  &llvm_java_release_double_array_elements,
  NULL,
  NULL, /* 200 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 210 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 220 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 230 */
  NULL,
};

const JNIEnv llvm_java_JNIEnv = &llvm_java_JNINativeInterface;

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
