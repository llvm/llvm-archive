import java.util.*;

public class Util
{
    private static Random rand = new Random(0);

    public static void randomFill(boolean[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(boolean[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = rand.nextBoolean();
    }

    public static void randomFill(byte[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(byte[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = (byte) rand.nextInt();
    }

    public static void randomFill(char[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(char[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = (char) rand.nextInt();
    }

    public static void randomFill(double[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(double[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = rand.nextDouble();
    }

    public static void randomFill(float[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(float[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = rand.nextFloat();
    }

    public static void randomFill(int[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(int[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = rand.nextInt();
    }

    public static void randomFill(long[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(long[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = rand.nextLong();
    }

    public static void randomFill(short[] a) {
        randomFill(a, 0, a.length);
    }

    public static void randomFill(short[] a, int from, int to) {
        for (; from < to; ++from)
            a[from] = (short) rand.nextInt();
    }

    public static void printlnElements(boolean[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(boolean[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(byte[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(byte[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(char[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(char[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(double[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(double[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(float[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(float[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(int[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(int[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(long[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(long[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void printlnElements(short[] a) {
        printlnElements(a, 0, a.length);
    }

    public static void printlnElements(short[] a, int from, int to) {
        for (; from < to; ++from)
            Test.println(a[from]);
    }

    public static void main(String[] args) {
        boolean[] az = new boolean[rand.nextInt(100)];
        randomFill(az);
        printlnElements(az);

        byte[] ab = new byte[rand.nextInt(100)];
        randomFill(ab);
        printlnElements(ab);

        char[] ac = new char[rand.nextInt(100)];
        randomFill(ac);
        printlnElements(ac);

        double[] ad = new double[rand.nextInt(100)];
        randomFill(ad);
        printlnElements(ad);

        float[] af = new float[rand.nextInt(100)];
        randomFill(af);
        printlnElements(af);

        int[] ai = new int[rand.nextInt(100)];
        randomFill(ai);
        printlnElements(ai);

        long[] al = new long[rand.nextInt(100)];
        randomFill(al);
        printlnElements(al);

        short[] as = new short[rand.nextInt(100)];
        randomFill(as);
        printlnElements(as);
    }
}
