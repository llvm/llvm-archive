public class Test
{
    static {
        System.loadLibrary("test");
    }

    public static native void print_boolean_ln(boolean b);
    public static native void print_int_ln(int i);
    public static native void print_long_ln(long l);
    public static native void print_float_ln(float f);
    public static native void print_double_ln(double d);

    public static void main(String[] args) {
        print_boolean_ln(true);
        print_boolean_ln(false);
        print_int_ln(123);
        print_int_ln(-321);
        print_long_ln(1234567890123456789L);
        print_long_ln(-1234567890123456789L);
        print_float_ln(753.46F);
        print_float_ln(-753.46F);
        print_double_ln(753.46);
        print_double_ln(-753.46);
    }
}
