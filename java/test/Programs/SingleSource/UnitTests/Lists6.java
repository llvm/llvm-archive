import java.util.*;

public class Lists6
{
    public static void main(String[] args) {
        List llist = new ArrayList();
        for (int i = 0; i < 15; ++i) {
            llist.add(new Integer(i));
        }

        for (Iterator i = llist.iterator(); i.hasNext(); ) {
            Integer I = (Integer) i.next();
            Test.println(I.intValue());
        }
    }
}
