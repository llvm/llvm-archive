import java.util.*;

public class Lists1
{
    public static void main(String[] args) {
        List list = new LinkedList();
        for (int i = 0; i < 17; ++i) {
            list.add(new Integer(i));
        }

        Test.println(list.isEmpty());
        Object[] array = list.toArray();

        for (int i = 0; i < array.length; ++i) {
            Test.println(((Integer) array[i]).equals(list.get(i)));
        }

        list.clear();

        Test.println(list.isEmpty());
    }
}
