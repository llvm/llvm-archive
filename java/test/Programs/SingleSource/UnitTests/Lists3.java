import java.util.*;

public class Lists3
{
    public static void main(String[] args) {
        LinkedList llist = new LinkedList();
        for (int i = 0; i < 10; ++i) {
            llist.add(new Integer(i));
        }

        Test.println(llist.contains(new Integer(5)));
        Test.println(llist.getFirst().equals(new Integer(0)));
        Test.println(llist.getLast().equals(new Integer(9)));
        Test.println(llist.indexOf(new Integer(3)));
        Test.println(llist.remove(new Integer(3)));
        Test.println(llist.indexOf(new Integer(3)));
        Test.println(((Integer)llist.removeFirst()).intValue());
        Test.println(((Integer)llist.removeLast()).intValue());
        Test.println(llist.size());

        for (int i = 0; i < llist.size(); ++i) {
            Test.println(((Integer)llist.get(i)).intValue());
        }
    }
}
