class VTableBase
{
    public int foo() { return 0; }
    public int bar() { return 0; }
}

interface VTableInterface
{
    public int baz();
}

public class VTable extends VTableBase implements VTableInterface
{
    public int foo() { return 1; }
    public int baz() { return 2; }

    public static void main(String[] args) {
        VTableBase a = new VTableBase();
        a.foo();
        a.bar();

        a = new VTable();
        a.foo();
        a.bar();
        ((VTableInterface)a).baz();

        VTableInterface i = new VTable();
        i.baz();
    }
}
