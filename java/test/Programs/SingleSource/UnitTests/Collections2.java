import java.util.*;

public class Collections2
{
    public static void main(String[] args) {
        Collection c1 = new TreeSet();
        for (int i = 0; i < 100; ++i) {
            c1.add(new Integer(i));
        }

        for (Iterator i = c1.iterator(); i.hasNext(); ) {
            Test.println(((Integer) i.next()).intValue());
        }
    }
}
