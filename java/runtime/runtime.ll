%llvm_java_base = type { %llvm_java_vtable_base* }
%llvm_java_vtable_base = type { %llvm_java_type_info }
%llvm_java_type_info = type { }

implementation

declare void %llvm_java_static_init()
declare void %llvm_java_main(int, sbyte**)

int %main(int %argc, sbyte** %argv) {
entry:
        call void %llvm_java_static_init()
        call void %llvm_java_main(int %argc, sbyte** %argv)
        ret int 0
}
