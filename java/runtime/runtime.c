struct llvm_java_object_base;
struct llvm_java_object_header;
struct llvm_java_object_vtable;
struct llvm_java_object_typeinfo;

/* Define VM internal types */
typedef struct llvm_java_object_base* jobject;
typedef unsigned jfieldID;
typedef unsigned jmethodID;
#define _JNI_VM_INTERNAL_TYPES_DEFINED

/* Define some types*/
/* FIXME: this should really be picked up from jni_md.h */
typedef unsigned char jboolean;
typedef signed char jbyte;
typedef unsigned short jchar;
typedef short jshort;
typedef int jint;
typedef long long jlong;
typedef float jfloat;
typedef double jdouble;

/* Used for jboolean type  */
/* FIXME: this should really be picked up from jni.h */
#define JNI_TRUE  1
#define JNI_FALSE 0

struct llvm_java_object_header {
  /* gc info, hash info, locking */
};

struct llvm_java_object_base {
  struct llvm_java_object_header header;
  struct llvm_java_object_vtable* vtable;
};

struct llvm_java_object_typeinfo {
  unsigned depth;
  struct llvm_java_object_vtable** vtables;
  unsigned lastIface;
  union {
    unsigned interfaceFlag;
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
  if (clazz->typeinfo.interfaceFlag != (unsigned)-1)
    return objClazz->typeinfo.depth > clazz->typeinfo.depth &&
           objClazz->typeinfo.vtables[objClazz->typeinfo.depth - clazz->typeinfo.depth - 1] == clazz;
  /* otherwise we are checking against an interface's typeinfo */
  else
    return objClazz->typeinfo.lastIface >= clazz->typeinfo.lastIface &&
           objClazz->typeinfo.interfaces[clazz->typeinfo.lastIface];
}


extern void llvm_java_static_init(void);
extern void llvm_java_main(int, char**);

int main(int argc, char** argv) {
  llvm_java_static_init();
  llvm_java_main(argc, argv);
  return 0;
}
