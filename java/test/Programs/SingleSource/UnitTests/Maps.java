import java.util.*;

public class Maps
{
    public static Random rand = new Random(0);

    public static void main(String[] args) {
        TreeMap tmap = new TreeMap();
        for (int i = 0; i < 1000; ++i)
            tmap.put(new Integer(i), new Integer(rand.nextInt()));
        for (int i = 0; i < 1000; ++i)
            Test.println(((Integer)tmap.get(new Integer(i))).intValue());
    }
}
