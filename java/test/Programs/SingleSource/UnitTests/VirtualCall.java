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
        Test.println(a.foo());
        Test.println(a.bar());

        a = new VirtualCallDerived();
        Test.println(a.foo());
        Test.println(a.bar());

        VirtualCallDerived b = new VirtualCallDerived();
        Test.println(b.foo());
        Test.println(b.bar());
    }
}
