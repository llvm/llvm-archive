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
}
