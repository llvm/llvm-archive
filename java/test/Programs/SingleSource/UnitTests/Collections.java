import java.util.*;

public class Collections
{
    public static void fillCollectionWithInts(Collection c) {
        for (int i = 0; i < 10; ++i)
            c.add(new Integer(i));
    }

    public static void printIntCollection(Collection c) {
        for (Iterator i = c.iterator(); i.hasNext(); )
            Test.println(((Integer)i.next()).intValue());
    }

    public static void main(String[] args) {
        Collection c1 = new TreeSet();
        fillCollectionWithInts(c1);
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
