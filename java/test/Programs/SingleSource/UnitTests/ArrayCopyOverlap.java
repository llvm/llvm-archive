import java.util.*;

public class ArrayCopyOverlap
{
    public static Random rand = new Random(0);

    public static void main(String[] args) {
        byte[] array = new byte[100];

        for (int i = 0; i < array.length; ++i) {
            array[i] = (byte) rand.nextInt();
        }

        System.arraycopy(array, 0, array, 23, 59);

        Util.printlnElements(array);
    }
}
