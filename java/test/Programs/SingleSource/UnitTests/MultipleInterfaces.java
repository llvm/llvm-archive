public class MultipleInterfaces
{
    interface I1
    {
        public int i1();
    }

    interface I2
    {
        public int i2();
    }

    interface I3 extends I1, I2
    {
        public int i3();
    }

    class C1 implements I3
    {
        public int i1() { return 1; }
        public int i2() { return 2; }
        public int i3() { return 3; }
    }

    public static void main(String[] args) {
        C1 o = new C1();
        I1 i1 = o;
        I2 i2 = o;
        I3 i3 = o;

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
