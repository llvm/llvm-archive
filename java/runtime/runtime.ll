"java/lang/Object" = type { }

implementation

declare void %llvm_java_static_init()
declare int %llvm_java_main(int, sbyte**)

int %main(int %argc, sbyte** %argv) {
entry:
        call void %llvm_java_static_init()
        %result = call int %llvm_java_main(int %argc, sbyte** %argv)
        ret int %result
}
