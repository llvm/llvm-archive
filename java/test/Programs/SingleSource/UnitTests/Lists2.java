import java.util.*;

public class Lists2
{
    public static void main(String[] args) {
        List llist = new LinkedList();
        Test.println(llist.isEmpty());

        for (int i = 0; i < 10; ++i) {
            llist.add(new Integer(i));
        }
        Test.println(llist.isEmpty());

        while (!llist.isEmpty()) {
            Integer I = (Integer) llist.remove(0);
            Test.println(I.intValue());
        }
        Test.println(llist.isEmpty());
    }
}
