public class Test
{
    public boolean z = false;
    public int i = 123;
    public long l = 1234567890123456789L;
    public float f = 753.46F;
    public double d = -46.75346;
    public short s = 456;
    public byte b = 78;

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

        new Test().printFields();
    }
}
