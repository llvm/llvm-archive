import java.util.*;

public class Maps1
{
    public static Random rand = new Random(0);

    public static void main(String[] args) {
        HashMap tmap = new HashMap();
        for (int i = 0; i < 1000; ++i)
            tmap.put(new Integer(i), new Integer(rand.nextInt()));
        for (int i = 0; i < 1000; ++i)
            Test.println(((Integer)tmap.get(new Integer(i))).intValue());
    }
}
