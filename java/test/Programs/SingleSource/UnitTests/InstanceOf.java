class InstanceOfBase
{
    int foo() { return 0; }
    int bar() { return 0; }
}

interface InstanceOfInterface {
    public int baz();
}

public class InstanceOf extends InstanceOfBase implements InstanceOfInterface
{
    int foo() { return 1; }
    public int baz() { return foo(); }

    public static void main(String[] args) {
        InstanceOfBase aa = new InstanceOfBase();
        InstanceOfBase ab = new InstanceOf();
        InstanceOfInterface bb = new InstanceOf();

        Test.print_boolean_ln(aa instanceof InstanceOfBase);
        Test.print_boolean_ln(aa instanceof InstanceOf);
        Test.print_boolean_ln(aa instanceof InstanceOfInterface);

        Test.print_boolean_ln(ab instanceof InstanceOfBase);
        Test.print_boolean_ln(ab instanceof InstanceOf);
        Test.print_boolean_ln(ab instanceof InstanceOfInterface);

        Test.print_boolean_ln(bb instanceof InstanceOfBase);
        Test.print_boolean_ln(bb instanceof InstanceOf);
        Test.print_boolean_ln(bb instanceof InstanceOfInterface);
    }
}
