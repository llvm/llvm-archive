import java.util.*;

public class Collections5
{
    public static void main(String[] args) {
        List l1 = new ArrayList();
        Collections.addRandomIntsToCollection(l1);
        Collections.printIntCollection(l1);
        java.util.Collections.rotate(l1, Collections.rand.nextInt(50000));
        Collections.printIntCollection(l1);
        java.util.Collections.shuffle(l1, Collections.rand);
        Collections.printIntCollection(l1);
        java.util.Collections.sort(l1);
        Collections.printIntCollection(l1);
    }
}
