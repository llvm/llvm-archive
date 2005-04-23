#include "runtime.h"
#include <stdlib.h>
#include <string.h>

/* The implementation of JNI functions */

/* Class operations */

static jclass find_class(JNIEnv* env, const char* name) {
  return GET_CLASS(llvm_java_find_class_record(name));
}

static jclass get_superclass(JNIEnv* env, jclass clazz) {
  return GET_CLASS(llvm_java_get_superclass_record(GET_CLASS_RECORD(clazz)));
}

static jboolean is_assignable_from(JNIEnv* env, jclass c1, jclass c2) {
  return llvm_java_is_assignable_from(GET_CLASS_RECORD(c1),
                                      GET_CLASS_RECORD(c2));
}

/* Exceptions */

static jint throw(JNIEnv* env, jthrowable obj) {
  return llvm_java_throw(obj);
}

/* Global and local references */

/* Weak global references */

/* Object operations */

static jboolean is_same_object(JNIEnv* env, jobject o1, jobject o2) {
  return o1 == o2;
}

static jclass get_object_class(JNIEnv* env, jobject obj) {
  return GET_CLASS(llvm_java_get_class_record(obj));
}

static jboolean is_instance_of(JNIEnv* env, jobject obj, jclass c) {
  return llvm_java_is_instance_of(obj, GET_CLASS_RECORD(c));
}

/* Accessing fields of objects */

static jfieldID get_fieldid(JNIEnv *env,
                            jclass clazz,
                            const char *name,
                            const char *sig) {
  int nameLength;
  int i;
  const char* fieldDescriptor;
  struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz);

  /* lookup the name+sig in the fieldDescriptors array and retrieve
   * the offset of the field */
  nameLength = strlen(name);
  for (i = 0; (fieldDescriptor = cr->typeinfo.fieldDescriptors[i]); ++i)
    if (strncmp(name, fieldDescriptor, nameLength) == 0 &&
        strcmp(sig, fieldDescriptor+nameLength) == 0)
      return cr->typeinfo.fieldOffsets[i];

  return 0;
}

#define HANDLE_TYPE(TYPE) \
  static j##TYPE get_##TYPE##_field(JNIEnv* env, \
                                    jobject obj, \
                                    jfieldID fid) { \
    return *(j##TYPE*) (((char*)obj) + fid); \
  }
#include "types.def"

#define HANDLE_TYPE(TYPE) \
  static void set_##TYPE##_field(JNIEnv* env, \
                                 jobject obj, \
                                 jfieldID fid, \
                                 j##TYPE value) { \
    *(j##TYPE*) (((char*)obj) + fid) = value; \
  }
#include "types.def"

/* Calling instance methods */

static jmethodID get_methodid(JNIEnv *env,
                              jclass clazz,
                              const char *name,
                              const char *sig) {
  int nameLength;
  int i;
  const char* methodDescriptor;
  struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz);

  /* lookup the name+sig in the staticFieldDescriptors array and
   * retrieve the index of the field */
  nameLength = strlen(name);
  for (i = 0; (methodDescriptor = cr->typeinfo.methodDescriptors[i]); ++i)
    if (strncmp(name, methodDescriptor, nameLength) == 0 &&
        strcmp(sig, methodDescriptor+nameLength) == 0)
      return i;

  return 0;
}

static void call_void_method_v(JNIEnv* env,
                               jobject obj,
                               jmethodID methodID,
                               va_list args) {
  typedef void (*BridgeFunPtr)(jobject obj, va_list);
  struct llvm_java_class_record* cr = llvm_java_get_class_record(obj);

  BridgeFunPtr f = (BridgeFunPtr) cr->typeinfo.methodBridges[methodID];
  f(obj, args);
}

static void call_void_method(JNIEnv* env,
                             jobject obj,
                             jmethodID methodID,
                             ...) {
  va_list args;
  va_start(args, methodID);
  call_void_method_v(env, obj, methodID, args);
  va_end(args);
}

#define HANDLE_TYPE(TYPE) \
  static j##TYPE call_##TYPE##_method_v(JNIEnv* env, \
                                        jobject obj, \
                                        jmethodID methodID, \
                                        va_list args) { \
    typedef j##TYPE (*BridgeFunPtr)(jobject obj, va_list); \
    struct llvm_java_class_record* cr = llvm_java_get_class_record(obj); \
    BridgeFunPtr f = \
      (BridgeFunPtr) cr->typeinfo.methodBridges[methodID]; \
    return f(obj, args); \
  }
#include "types.def"

#define HANDLE_TYPE(TYPE) \
  static j##TYPE call_##TYPE##_method(JNIEnv* env, \
                                      jobject obj, \
                                      jmethodID methodID, \
                                      ...) { \
    va_list args; \
    va_start(args, methodID); \
    j##TYPE result = \
      call_##TYPE##_method_v(env, obj, methodID, args); \
    va_end(args); \
    return result; \
  }
#include "types.def"

/* Accessing static fields */

static jfieldID get_static_fieldid(JNIEnv *env,
                                   jclass clazz,
                                   const char *name,
                                   const char *sig) {
  int nameLength;
  int i;
  const char* fieldDescriptor;
  struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz);

  /* lookup the name+sig in the staticFieldDescriptors array and
   * retrieve the index of the field */
  nameLength = strlen(name);
  for (i = 0; (fieldDescriptor = cr->typeinfo.staticFieldDescriptors[i]); ++i)
    if (strncmp(name, fieldDescriptor, nameLength) == 0 &&
        strcmp(sig, fieldDescriptor+nameLength) == 0)
      return i;

  return 0;
}

#define HANDLE_TYPE(TYPE) \
  static j##TYPE get_static_##TYPE##_field(JNIEnv* env, \
                                           jclass clazz, \
                                           jfieldID fid) { \
    struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz); \
    return *(j##TYPE*) cr->typeinfo.staticFields[fid]; \
  }
#include "types.def"

#define HANDLE_TYPE(TYPE) \
  static void set_static_##TYPE##_field(JNIEnv* env, \
                                        jclass clazz, \
                                        jfieldID fid, \
                                        j##TYPE value) { \
    struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz); \
    *(j##TYPE*) cr->typeinfo.staticFields[fid] = value; \
  }
#include "types.def"

/* Calling static methods */

static jmethodID get_static_methodid(JNIEnv *env,
                                     jclass clazz,
                                     const char *name,
                                     const char *sig) {
  int nameLength;
  int i;
  const char* methodDescriptor;
  struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz);

  /* lookup the name+sig in the staticFieldDescriptors array and
   * retrieve the index of the field */
  nameLength = strlen(name);
  for (i = 0; (methodDescriptor = cr->typeinfo.staticMethodDescriptors[i]); ++i)
    if (strncmp(name, methodDescriptor, nameLength) == 0 &&
        strcmp(sig, methodDescriptor+nameLength) == 0)
      return i;

  return 0;
}

static void call_static_void_method_v(JNIEnv* env,
                                      jclass clazz,
                                      jmethodID methodID,
                                      va_list args) {
  typedef void (*BridgeFunPtr)(jclass clazz, va_list);
  struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz);

  BridgeFunPtr f = (BridgeFunPtr) cr->typeinfo.staticMethodBridges[methodID];
  f(clazz, args);
}

static void call_static_void_method(JNIEnv* env,
                                    jclass clazz,
                                    jmethodID methodID,
                                    ...) {
  va_list args;
  va_start(args, methodID);
  call_static_void_method_v(env, clazz, methodID, args);
  va_end(args);
}

#define HANDLE_TYPE(TYPE) \
  static j##TYPE call_static_##TYPE##_method_v(JNIEnv* env, \
                                               jclass clazz, \
                                               jmethodID methodID, \
                                               va_list args) { \
    typedef j##TYPE (*BridgeFunPtr)(jclass clazz, va_list); \
    struct llvm_java_class_record* cr = GET_CLASS_RECORD(clazz); \
    BridgeFunPtr f = \
      (BridgeFunPtr) cr->typeinfo.staticMethodBridges[methodID]; \
    return f(clazz, args); \
  }
#include "types.def"

#define HANDLE_TYPE(TYPE) \
  static j##TYPE call_static_##TYPE##_method(JNIEnv* env, \
                                             jclass clazz, \
                                             jmethodID methodID, \
                                             ...) { \
    va_list args; \
    va_start(args, methodID); \
    j##TYPE result = \
      call_static_##TYPE##_method_v(env, clazz, methodID, args); \
    va_end(args); \
    return result; \
  }
#include "types.def"

/* String operations */

/* Array operations */

static jint get_array_length(JNIEnv* env, jarray array) {
  return ((struct llvm_java_booleanarray*) array)->length;
}

static jobject get_object_array_element(JNIEnv* env, jarray array, jsize i) {
  return ((struct llvm_java_objectarray*) array)->data[i];
}

static void set_object_array_element(JNIEnv* env,
                                     jarray array,
                                     jsize i,
                                     jobject value) {
  ((struct llvm_java_objectarray*) array)->data[i] = value;
}

#define HANDLE_NATIVE_TYPE(TYPE) \
  static j ## TYPE* get_##TYPE##_array_elements( \
    JNIEnv* env, \
    jarray array, \
    jboolean* isCopy) { \
    if (isCopy) \
      *isCopy = JNI_FALSE; \
    return ((struct llvm_java_ ##TYPE## array*) array)->data; \
  }
#include "types.def"

#define HANDLE_NATIVE_TYPE(TYPE) \
  static void release_ ##TYPE## _array_elements( \
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

/* Register native methods */

/* Monitor operations */

/* NIO support */

/* Reflection support */

/* Java VM interface */

/* The JNI interface definition */
static const struct JNINativeInterface llvm_java_JNINativeInterface = {
  NULL, /* 0 */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &find_class,
  NULL,
  NULL,
  NULL,
  &get_superclass,
  &is_assignable_from,
  NULL,
  &throw,
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
  &is_same_object,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 30 */
  &get_object_class,
  &is_instance_of,
  &get_methodid,
  &call_object_method,
  &call_object_method_v,
  NULL,
  &call_boolean_method,
  &call_boolean_method_v,
  NULL,
  &call_byte_method,
  &call_byte_method_v,
  NULL,
  &call_char_method,
  &call_char_method_v,
  NULL,
  &call_short_method,
  &call_short_method_v,
  NULL,
  &call_int_method,
  &call_int_method_v,
  NULL,
  &call_long_method,
  &call_long_method_v,
  NULL,
  &call_float_method,
  &call_float_method_v,
  NULL,
  &call_double_method,
  &call_double_method_v,
  NULL, /* 60 */
  &call_void_method,
  &call_void_method_v,
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
  &get_fieldid,
  &get_object_field,
  &get_boolean_field,
  &get_byte_field,
  &get_char_field,
  &get_short_field,
  &get_int_field,
  &get_long_field,
  &get_float_field,
  &get_double_field,
  &set_object_field,
  &set_boolean_field,
  &set_byte_field,
  &set_char_field,
  &set_short_field,
  &set_int_field,
  &set_long_field,
  &set_float_field,
  &set_double_field,
  &get_static_methodid,
  &call_static_object_method,
  &call_static_object_method_v,
  NULL,
  &call_static_boolean_method,
  &call_static_boolean_method_v,
  NULL,
  &call_static_byte_method,
  &call_static_byte_method_v,
  NULL,
  &call_static_char_method,
  &call_static_char_method_v,
  NULL,
  &call_static_short_method,
  &call_static_short_method_v,
  NULL,
  &call_static_int_method,
  &call_static_int_method_v,
  NULL,
  &call_static_long_method,
  &call_static_long_method_v,
  NULL,
  &call_static_float_method,
  &call_static_float_method_v,
  NULL,
  &call_static_double_method,
  &call_static_double_method_v,
  NULL, /* 140 */
  NULL,
  NULL,
  NULL,
  &get_static_fieldid,
  &get_static_object_field,
  &get_static_boolean_field,
  &get_static_byte_field,
  &get_static_char_field,
  &get_static_short_field,
  &get_static_int_field,
  &get_static_long_field,
  &get_static_float_field,
  &get_static_double_field,
  &set_static_object_field,
  &set_static_boolean_field,
  &set_static_byte_field,
  &set_static_char_field,
  &set_static_short_field,
  &set_static_int_field,
  &set_static_long_field,
  &set_static_float_field,
  &set_static_double_field,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 170 */
  &get_array_length,
  NULL,
  &get_object_array_element,
  &set_object_array_element,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL, /* 180 */
  NULL,
  NULL,
  &get_boolean_array_elements,
  &get_byte_array_elements,
  &get_char_array_elements,
  &get_short_array_elements,
  &get_int_array_elements,
  &get_long_array_elements,
  &get_float_array_elements,
  &get_double_array_elements,
  &release_boolean_array_elements,
  &release_byte_array_elements,
  &release_char_array_elements,
  &release_short_array_elements,
  &release_int_array_elements,
  &release_long_array_elements,
  &release_float_array_elements,
  &release_double_array_elements,
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
