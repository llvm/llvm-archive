"<llvm_java_base>" = type "<llvm_java_vtable_base>"*
"<llvm_java_vtable_base>" = type { "<llvm_java_type_info>" }
"<llvm_java_type_info>" = type { "<llvm_java_vtable_base>"* }

implementation

declare void %llvm_java_static_init()
declare void %llvm_java_main(int, sbyte**)

int "<llvm_java_issubclass>"("<llvm_java_vtable_base>"* %self,
                             "<llvm_java_vtable_base>"* %class) {
      %eq = seteq "<llvm_java_vtable_base>"* %self, %class
      br bool %eq, label %isEqual, label %isNotEqual
isEqual:
      ret int 1
isNotEqual:
      %superPtr = getelementptr "<llvm_java_vtable_base>"* %self, uint 0, uint 0, uint 0
      %super = load "<llvm_java_vtable_base>"** %superPtr
      %nu = seteq "<llvm_java_vtable_base>"* %self, null
      br bool %nu, label %isNull, label %isNotNull
isNull:
      ret int 0
isNotNull:
      %res = call int("<llvm_java_vtable_base>"*,"<llvm_java_vtable_base>"*)* "<llvm_java_issubclass>"("<llvm_java_vtable_base>"* %super, "<llvm_java_vtable_base>"* %class)
      ret int %res
}

int "<llvm_java_instanceof>"("<llvm_java_vtable_base>"* %self,
                             "<llvm_java_vtable_base>"* %class) {
       %c = seteq "<llvm_java_vtable_base>"* %self, null
       br bool %c, label %isNull, label %isNotNull
isNull:
       ret int 0
isNotNull:
       %res = call int("<llvm_java_vtable_base>"*,"<llvm_java_vtable_base>"*)* "<llvm_java_issubclass>"("<llvm_java_vtable_base>"* %self, "<llvm_java_vtable_base>"* %class)
      ret int %res
}

int %main(int %argc, sbyte** %argv) {
entry:
        call void %llvm_java_static_init()
        call void %llvm_java_main(int %argc, sbyte** %argv)
        ret int 0
}
