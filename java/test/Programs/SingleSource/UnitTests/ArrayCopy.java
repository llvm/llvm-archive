import java.util.*;

public class ArrayCopy
{
    public static Random rand = new Random(0);

    public static int min(int a, int b) {
        return a < b ? a : b;
    }

    public static void fillArray(Object dst, int dstLength, Object src, int srcLength) {
        for (int i = 0; i < dstLength; i += srcLength)
            System.arraycopy(src, 0, dst, i, min(dstLength - i, srcLength));
    }

    public static void main(String[] args) {
        int[] isrc = new int[10];
        byte[] bsrc = new byte[10];
        float[] fsrc = new float[10];
        double[] dsrc = new double[10];
        boolean[] zsrc = new boolean[10];
        long[] lsrc = new long[10];
        Object[] osrc = new Object[10];

        for (int i = 0; i < 10; ++i) {
            isrc[i] = rand.nextInt();
            bsrc[i] = (byte) rand.nextInt();
            fsrc[i] = rand.nextFloat();
            dsrc[i] = rand.nextDouble();
            zsrc[i] = rand.nextBoolean();
            lsrc[i] = rand.nextLong();
            osrc[i] = new Integer(rand.nextInt());
        }

        int[] idst = new int[7];
        byte[] bdst = new byte[19];
        float[] fdst = new float[28];
        double[] ddst = new double[23];
        boolean[] zdst = new boolean[17];
        long[] ldst = new long[6];
        Object[] odst = new Object[22];

        fillArray(idst, idst.length, isrc, isrc.length);
        fillArray(bdst, bdst.length, bsrc, bsrc.length);
        fillArray(fdst, fdst.length, fsrc, fsrc.length);
        fillArray(ddst, ddst.length, dsrc, dsrc.length);
        fillArray(zdst, zdst.length, zsrc, zsrc.length);
        fillArray(ldst, ldst.length, lsrc, lsrc.length);
        fillArray(odst, odst.length, osrc, osrc.length);

        for (int i = 0; i < idst.length; ++i)
            Test.println(idst[i]);
        for (int i = 0; i < bdst.length; ++i)
            Test.println(bdst[i]);
        for (int i = 0; i < fdst.length; ++i)
            Test.println(fdst[i]);
        for (int i = 0; i < ddst.length; ++i)
            Test.println(ddst[i]);
        for (int i = 0; i < zdst.length; ++i)
            Test.println(zdst[i]);
        for (int i = 0; i < ldst.length; ++i)
            Test.println(ldst[i]);
        for (int i = 0; i < odst.length; ++i)
            Test.println(((Integer)odst[i]).intValue());
    }
}
