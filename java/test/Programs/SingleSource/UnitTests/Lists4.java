import java.util.*;

public class Lists4
{
    public static void main(String[] args) {
        LinkedList llist = new LinkedList();
        llist.add(null);
        Test.println(llist.getFirst() == null);
        Test.println(llist.contains(null));
        Test.println(llist.get(0) == null);
    }
}
