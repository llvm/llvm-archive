public class StaticInitializers
{
    static int foo = 0;
    static int bar = init();
    static int baz = init();

     static int init() {
        return ++foo;
    }

    public static void main(String[] args) {
        Test.println(foo);
        Test.println(bar);
        Test.println(baz);
    }
}
