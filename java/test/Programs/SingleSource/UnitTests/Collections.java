import java.util.*;

public class Collections
{
    public static void main(String[] args) {
        Collection c1 = new LinkedList();
        for (int i = 0; i < 100; ++i) {
            c1.add(new Integer(i));
        }
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
