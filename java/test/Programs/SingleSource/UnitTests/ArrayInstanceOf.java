public class ArrayInstanceOf
{
    private static class Base
    {
        int foo() { return 0; }
        int bar() { return 0; }
    }

    interface Interface
    {
        public int baz();
    }

    private static class Derived extends Base implements Interface
    {
        int foo() { return 1; }
        public int baz() { return foo(); }

    }

    public static void main(String[] args) {
	Object o = new Object[10];
        Object b = new Base[10];
        Object d = new Derived[10];
        Object i = new Interface[10];

        Test.println(o instanceof Object[]);
        Test.println(o instanceof Base[]);
        Test.println(o instanceof Interface[]);
        Test.println(o instanceof Derived[]);

        Test.println(b instanceof Object[]);
        Test.println(b instanceof Base[]);
        Test.println(b instanceof Interface[]);
        Test.println(b instanceof Derived[]);

        Test.println(d instanceof Object[]);
        Test.println(d instanceof Base[]);
        Test.println(d instanceof Interface[]);
        Test.println(d instanceof Derived[]);

        Test.println(i instanceof Object[]);
        Test.println(i instanceof Base[]);
        Test.println(i instanceof Interface[]);
        Test.println(i instanceof Derived[]);
    }
}
