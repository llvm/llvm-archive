public class StaticInitializers
{
    static int foo = 0;
    static int bar = init();
    static int baz = init();

     static int init() {
        return ++foo;
    }

    public static void main(String[] args) {
        System.out.println("foo = " + foo);
        System.out.println("bar = " + bar);
        System.out.println("baz = " + baz);
    }
}
