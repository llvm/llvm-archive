import java.util.*;

public class Collections5
{
    public static void testList(List l) {
        Collections.addRandomIntsToCollection(l);
        Collections.printIntCollection(l);
        java.util.Collections.rotate(l, Collections.rand.nextInt(50000));
        Collections.printIntCollection(l);
        java.util.Collections.shuffle(l, Collections.rand);
        Collections.printIntCollection(l);
        java.util.Collections.sort(l);
        Collections.printIntCollection(l);
    }

    public static void main(String[] args) {
        testList(new LinkedList());
        testList(new ArrayList());
    }
}
