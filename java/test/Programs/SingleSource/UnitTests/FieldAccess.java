public class FieldAccess
{
    private static class Base {
        int i;
        float f;
    }

    private static class Derived extends Base {
        int i;
        double d;
    }

    public static void main(String[] args) {
        Derived b = new Derived();
        b.d = 4.0;
        b.i = 3;
        b.f = 2.0F;
        ((Base) b).i = 4;
        ((Base) b).f = 1.0F;

        Test.print_int_ln(((Base)b).i);
        Test.print_int_ln(b.i);
        Test.print_float_ln(((Base)b).f);
        Test.print_float_ln(b.f);
        Test.print_double_ln(b.d);
    }
}
