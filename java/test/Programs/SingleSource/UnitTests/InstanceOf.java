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

        Test.print_boolean_ln(aa instanceof Base);
        Test.print_boolean_ln(aa instanceof Derived);
        Test.print_boolean_ln(aa instanceof Interface);

        Test.print_boolean_ln(ab instanceof Base);
        Test.print_boolean_ln(ab instanceof Derived);
        Test.print_boolean_ln(ab instanceof Interface);

        Test.print_boolean_ln(bb instanceof Base);
        Test.print_boolean_ln(bb instanceof Derived);
        Test.print_boolean_ln(bb instanceof Interface);
    }
}
