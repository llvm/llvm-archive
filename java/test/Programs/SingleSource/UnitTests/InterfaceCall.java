class InterfaceCallBase implements InterfaceCallInterface
{
    public int foo() { return 1; }
    public int bar() { return 2; }
}

class InterfaceCallDerived extends InterfaceCallBase
{
    public int foo() { return 100; }
    public int bar() { return super.bar() + super.foo(); }
}

interface InterfaceCallInterface
{
    public int bar();
}

public class InterfaceCall
{
    public static void main(String[] args) {
        InterfaceCallInterface i = new InterfaceCallBase();
        Test.print_int_ln(i.bar());

        i = new InterfaceCallDerived();
        Test.print_int_ln(i.bar());
    }
}
