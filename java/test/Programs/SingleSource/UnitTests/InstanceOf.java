public class InstanceOf
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
        Base aa = new Base();
        Base ab = new Derived();
        Interface bb = new Derived();

        Test.println(aa instanceof Base);
        Test.println(aa instanceof Derived);
        Test.println(aa instanceof Interface);

        Test.println(ab instanceof Base);
        Test.println(ab instanceof Derived);
        Test.println(ab instanceof Interface);

        Test.println(bb instanceof Base);
        Test.println(bb instanceof Derived);
        Test.println(bb instanceof Interface);
    }
}
