struct llvm_java_object_base;
struct llvm_java_object_header;
struct llvm_java_object_vtable;
struct llvm_java_object_typeinfo;

struct llvm_java_object_header {
  // gc info, hash info, locking
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
llvm_java_GetObjectClass(struct llvm_java_object_base* obj) {
  return obj->vtable;
}

int llvm_java_IsInstanceOf(struct llvm_java_object_base* obj,
                           struct llvm_java_object_vtable* clazz) {
  struct llvm_java_object_vtable* objClazz = obj->vtable;
  if (objClazz == clazz)
    return 1;
  // we are checking against a class' typeinfo
  if (clazz->typeinfo.interfaceFlag != (unsigned)-1)
    return objClazz->typeinfo.depth > clazz->typeinfo.depth &&
           objClazz->typeinfo.vtables[objClazz->typeinfo.depth - clazz->typeinfo.depth - 1] == clazz;
  // otherwise we are checking against an interface's typeinfo
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
