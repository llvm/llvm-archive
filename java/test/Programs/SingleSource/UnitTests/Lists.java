import java.util.*;

public class Lists
{
    public static void main(String[] args) {
        LinkedList llist = new LinkedList();
        for (int i = 0; i < 10; ++i) {
            llist.add(new Integer(i));
        }

        while (!llist.isEmpty()) {
            Integer I = (Integer) llist.removeFirst();
            Test.print_int_ln(I.intValue());
        }
    }
}
