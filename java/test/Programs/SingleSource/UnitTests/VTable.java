class VTableBase
{
    public int foo() { return 0; }
    public int bar() { return 1; }
}

interface VTableInterface
{
    public int baz();
}

public class VTable extends VTableBase implements VTableInterface
{
    public int foo() { return 2; }
    public int baz() { return 3; }

    public static void main(String[] args) {
        VTableBase a = new VTableBase();
        Test.println(a.foo());
        Test.println(a.bar());

        a = new VTable();
        Test.println(a.foo());
        Test.println(a.bar());
        Test.println(((VTableInterface)a).baz());

        VTableInterface i = new VTable();
        Test.println(i.baz());
    }
}
