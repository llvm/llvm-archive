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
        Object[] ob = new Base[10];
        Object[] od = new Derived[10];
        Base[] bb = new Base[10];
        Base[] bd = new Derived[10];
        Interface[] id = new Derived[10];
        Derived[] dd = new Derived[10];

        Test.println(ob instanceof Object[]);
        Test.println(ob instanceof Base[]);
        Test.println(ob instanceof Interface[]);
        Test.println(ob instanceof Derived[]);

        Test.println(od instanceof Object[]);
        Test.println(od instanceof Base[]);
        Test.println(od instanceof Interface[]);
        Test.println(od instanceof Derived[]);

        Test.println(bb instanceof Object[]);
        Test.println(bb instanceof Base[]);
        Test.println(bb instanceof Interface[]);
        Test.println(bb instanceof Derived[]);

        Test.println(bd instanceof Object[]);
        Test.println(bd instanceof Base[]);
        Test.println(bd instanceof Interface[]);
        Test.println(bd instanceof Derived[]);

        Test.println(id instanceof Object[]);
        Test.println(id instanceof Base[]);
        Test.println(id instanceof Interface[]);
        Test.println(id instanceof Derived[]);

        Test.println(dd instanceof Object[]);
        Test.println(dd instanceof Base[]);
        Test.println(dd instanceof Interface[]);
        Test.println(dd instanceof Derived[]);

    }
}
