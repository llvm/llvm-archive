class VTableBase
{
    int foo() { return 0; }
    int bar() { return 0; }
}

public class VTable extends VTableBase
{
    int foo() { return 1; }

    public static void main(String[] args) {
        VTableBase a = new VTableBase();
        a.foo();
        a.bar();

        a = new VTable();
        a.foo();
        a.bar();
    }
}
