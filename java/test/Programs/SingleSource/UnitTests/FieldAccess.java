class FieldAccessBase {
    int i;
    float f;
}

public class FieldAccess extends FieldAccessBase
{
    int i;
    double d;

    public static void main(String[] args) {
        FieldAccess b = new FieldAccess();
        b.d = 4.0;
        b.i = 3;
        b.f = 2.0F;
        ((FieldAccessBase) b).i = 4;
        ((FieldAccessBase) b).f = 1.0F;

        Test.print_int_ln(((FieldAccessBase)b).i);
        Test.print_int_ln(b.i);
        Test.print_float_ln(((FieldAccessBase)b).f);
        Test.print_float_ln(b.f);
        Test.print_double_ln(b.d);
    }
}
