public class StaticInitializers
{
    static int foo = 0;
    static int bar = init();
    static int baz = init();

     static int init() {
        return ++foo;
    }

    public static void main(String[] args) {
        Test.print_int_ln(foo);
        Test.print_int_ln(bar);
        Test.print_int_ln(baz);
    }
}
