import java.util.*;

public class ArrayCopy
{
    public static int min(int a, int b) {
        return a < b ? a : b;
    }

    public static void main(String[] args) {
        int[] iarray1 = new int[10];
        for (int i = 0; i < iarray1.length; ++i)
            iarray1[i] = i;

        int[] iarray2 = new int[123];
        for (int i = 0; i < iarray2.length; i += iarray1.length)
            System.arraycopy(iarray1, 0, iarray2, i, min(iarray2.length - i, iarray1.length));

        for (int i = 0; i < iarray2.length; ++i)
            Test.println(iarray2[i]);
    }
}
