import java.util.*;

public class Collections
{
    public static Random rand = new Random(0);

    public static void addRandomIntsToCollection(Collection c) {
        int size = rand.nextInt(45) + 5;
        for (int i = 0; i < size; ++i)
            c.add(new Integer(rand.nextInt()));
    }

    public static void printIntCollection(Collection c) {
        for (Iterator i = c.iterator(); i.hasNext(); )
            Test.println(((Integer)i.next()).intValue());
    }

    public static void main(String[] args) {
        Collection c1 = new TreeSet();
        addRandomIntsToCollection(c1);
        Collection c2 = new TreeSet(c1);

        Test.println(c1.remove(new Integer(5)));
        Test.println(c1.equals(c2));
        Test.println(c2.remove(new Integer(5)));
        Test.println(c1.equals(c2));

        Test.println(c1.remove(new Integer(5)));
        Test.println(c1.equals(c2));
        Test.println(c2.remove(new Integer(5)));
        Test.println(c1.equals(c2));
    }
}
