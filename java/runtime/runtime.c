
#include <stdlib.h>
#include <llvm/Java/jni.h>

struct llvm_java_object_base;
struct llvm_java_object_header;
struct llvm_java_object_vtable;
struct llvm_java_object_typeinfo;

struct llvm_java_object_header {
  /* gc info, hash info, locking */
};

struct llvm_java_object_base {
  struct llvm_java_object_header header;
  struct llvm_java_object_vtable* vtable;
};

struct llvm_java_object_typeinfo {
  int depth;
  struct llvm_java_object_vtable** vtables;
  int lastIface;
  union {
    int interfaceFlag;
    struct llvm_java_object_vtable** interfaces;
  };
};

struct llvm_java_object_vtable {
  struct llvm_java_object_typeinfo typeinfo;
};

struct llvm_java_object_vtable*
llvm_java_GetObjectClass(jobject obj) {
  return obj->vtable;
}

void llvm_java_SetObjectClass(jobject obj,
                              struct llvm_java_object_vtable* clazz) {
  obj->vtable = clazz;
}

jint llvm_java_IsInstanceOf(jobject obj,
                            struct llvm_java_object_vtable* clazz) {
  /* trivial case 1: a null object can be cast to any type */
  if (!obj)
    return JNI_TRUE;

  struct llvm_java_object_vtable* objClazz = obj->vtable;
  /* trivial case 2: this object is of class clazz */
  if (objClazz == clazz)
    return JNI_TRUE;

  /* we are checking against a class' typeinfo */
  if (clazz->typeinfo.interfaceFlag != -1) {
    /* this class' vtable can only be found at this index */
    int index = objClazz->typeinfo.depth - clazz->typeinfo.depth - 1;
    return index >= 0 && objClazz->typeinfo.vtables[index] == clazz;
  }
  /* otherwise we are checking against an interface's typeinfo */
  else {
    /* this interface's vtable can only be found at this index */
    int index = clazz->typeinfo.lastIface;
    return objClazz->typeinfo.lastIface >= index &&
           objClazz->typeinfo.interfaces[index];
  }
}

jint llvm_java_Throw(jobject obj) {
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

extern void llvm_java_static_init(void);
extern void llvm_java_main(int, char**);

int main(int argc, char** argv) {
  llvm_java_static_init();
  llvm_java_main(argc, argv);
  return 0;
}
