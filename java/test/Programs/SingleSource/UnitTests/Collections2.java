import java.util.*;

public class Collections2
{
    public static void main(String[] args) {
        Collection c1 = new TreeSet();
        Collections.fillCollectionWithRandomInts(c1);
        Collections.printIntCollection(c1);
    }
}
