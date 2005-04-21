public class Test
{
    // Fields to test JNI getFieldID and get<TYPE>Field functions.
    public boolean z = false;
    public int i = 123;
    public long l = 1234567890123456789L;
    public float f = 753.46F;
    public double d = -46.75346;
    public short s = 456;
    public byte b = 78;

    // Fields to test JNI getStaticFieldID and getStatic<TYPE>Field functions.
    public static boolean Z = true;
    public static int I = 321;
    public static long L = 1234567890987654321L;
    public static float F = 46.753F;
    public static double D = -75346.46;
    public static short S = 654;
    public static byte B = 87;

    // Methods to test JNI getMethodID and call<TYPE>Method functions.
    public boolean z() { return z; }
    public int i(int ii) { return i + ii; }
    public long j(byte bb, short ss) { return l + bb + ss; }
    public float f(byte bb) { return f + bb; }
    public double d(int ii, long ll) { return d + ii + ll; }
    public short s(double dd, byte bb) { return (short) (s + dd + bb); }
    public byte b(short ss, float ff) { return (byte)(B + ff + ss); }

    // Methods to test JNI getStaticMethodID and
    // callStatic<TYPE>Method functions.
    public static boolean Z() { return Z; }
    public static int I(int jj) { return I + jj; }
    public static long J(byte bb, short ss) { return L + bb + ss; }
    public static float F(byte bb) { return F + bb; }
    public static double D(int ii, long ll) { return D + ii + ll; }
    public static short S(double dd, byte bb) { return (short) (S + dd + bb); }
    public static byte B(short ss, float ff) { return (byte)(B + ff + ss); }

    static {
        System.loadLibrary("test");
    }

    public static native void println(boolean b);
    public static native void println(int i);
    public static native void println(long l);
    public static native void println(float f);
    public static native void println(double d);
    public static void println(Object o) {
        println(o.toString());
    }
    public static void println(String s) {
        byte[] bytes = new byte[s.length()];
        s.getBytes(0, s.length(), bytes, 0);
        println(bytes);
    }
    private static native void println(byte[] a);

    public native void printFields();
    public static native void printStaticFields();
    public native void printMethods();
    public static native void printStaticMethods();

    public static void main(String[] args) {
        println(true);
        println(false);
        println(123);
        println(-321);
        println(1234567890123456789L);
        println(-1234567890123456789L);
        println(753.46F);
        println(-753.46F);
        println(753.46);
        println(-753.46);
        println(new byte[] { 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd' });
        println("Hello world");
        println("Printing static fields:");
        printStaticFields();
        println("Printing member fields:");
        new Test().printFields();
        println("Calling static methods:");
        printStaticMethods();
        println("Calling member methods:");
        new Test().printMethods();
    }
}
