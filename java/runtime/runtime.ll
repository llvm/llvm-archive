"java/lang/Object" = type { }

implementation

declare void %llvm_java_static_init()
declare void %llvm_java_main(int, sbyte**)

int %main(int %argc, sbyte** %argv) {
entry:
        call void %llvm_java_static_init()
        call void %llvm_java_main(int %argc, sbyte** %argv)
        ret int 0
}
