class VirtualCallBase
{
    public int foo() { return 1; }
    public int bar() { return 2; }
}

class VirtualCallDerived extends VirtualCallBase
{
    public int foo() { return 100; }
    public int bar() { return super.bar() + super.foo(); }
}

public class VirtualCall
{
    public static void main(String[] args) {
        VirtualCallBase a = new VirtualCallBase();
        Test.print_int_ln(a.foo());
        Test.print_int_ln(a.bar());

        a = new VirtualCallDerived();
        Test.print_int_ln(a.foo());
        Test.print_int_ln(a.bar());

        VirtualCallDerived b = new VirtualCallDerived();
        Test.print_int_ln(b.foo());
        Test.print_int_ln(b.bar());
    }
}
