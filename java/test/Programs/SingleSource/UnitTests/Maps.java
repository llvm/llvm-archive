import java.util.*;

public class Maps
{
    public static Random rand = new Random(0);

    public static void fillMapWithRandomInts(Map m) {
        int size = rand.nextInt(45) + 5;
        for (int i = 0; i < size; ++i)
            m.put(new Integer(i), new Integer(rand.nextInt()));
    }

    public static void printIntMap(Map m) {
        for (int i = 0; i < m.size(); ++i) {
            Integer I = (Integer) m.get(new Integer(i));
            Test.println(I.intValue());
        }
    }

    public static void main(String[] args) {
        TreeMap tmap = new TreeMap();
        fillMapWithRandomInts(tmap);
        printIntMap(tmap);
    }
}
