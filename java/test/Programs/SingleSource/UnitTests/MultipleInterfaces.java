interface MultipleInterfacesI1
{
    public int i1();
}

interface MultipleInterfacesI2
{
    public int i2();
}

interface MultipleInterfacesI3 extends MultipleInterfacesI1, MultipleInterfacesI2
{
    public int i3();
}

class MultipleInterfacesClass implements MultipleInterfacesI3
{
    public int i1() { return 3; }
    public int i2() { return 3; }
    public int i3() { return 3; }
}

public class MultipleInterfaces
{
    public static void main(String[] args) {
        MultipleInterfacesClass o = new MultipleInterfacesClass();
        MultipleInterfacesI1 i1 = o;
        MultipleInterfacesI2 i2 = o;
        MultipleInterfacesI3 i3 = o;

        Test.print_int_ln(o.i1());
        Test.print_int_ln(o.i2());
        Test.print_int_ln(o.i3());

        Test.print_int_ln(i1.i1());

        Test.print_int_ln(i2.i2());

        Test.print_int_ln(i3.i1());
        Test.print_int_ln(i3.i2());
        Test.print_int_ln(i3.i3());
    }
}
