class InstanceOfBase
{
    int foo() { return 0; }
    int bar() { return 0; }
}

interface InstanceOfInterface
{
    public int baz();
}

class InstanceOfDerived extends InstanceOfBase implements InstanceOfInterface
{
    int foo() { return 1; }
    public int baz() { return foo(); }

}

public class InstanceOf
{
    public static void main(String[] args) {
        InstanceOfBase aa = new InstanceOfBase();
        InstanceOfBase ab = new InstanceOfDerived();
        InstanceOfInterface bb = new InstanceOfDerived();

        Test.print_boolean_ln(aa instanceof InstanceOfBase);
        Test.print_boolean_ln(aa instanceof InstanceOfDerived);
        Test.print_boolean_ln(aa instanceof InstanceOfInterface);

        Test.print_boolean_ln(ab instanceof InstanceOfBase);
        Test.print_boolean_ln(ab instanceof InstanceOfDerived);
        Test.print_boolean_ln(ab instanceof InstanceOfInterface);

        Test.print_boolean_ln(bb instanceof InstanceOfBase);
        Test.print_boolean_ln(bb instanceof InstanceOfDerived);
        Test.print_boolean_ln(bb instanceof InstanceOfInterface);
    }
}
