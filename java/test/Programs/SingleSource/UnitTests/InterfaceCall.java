public class InterfaceCall
{
    private static class Base implements Interface
    {
        public int foo() { return 1; }
        public int bar() { return 2; }
    }

    private static class Derived extends Base
    {
        public int foo() { return 100; }
        public int bar() { return super.bar() + super.foo(); }
    }

    interface Interface
    {
        public int bar();
    }

    public static void main(String[] args) {
        Interface i = new Base();
        Test.print_int_ln(i.bar());

        i = new Derived();
        Test.print_int_ln(i.bar());
    }
}
