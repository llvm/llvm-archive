public class Equals
{
    public static void main(String[] args) {
        Object i1 = new Integer(123);
        Object i2 = new Integer(123);
        Object i3 = new Long(123);

        Test.println(i1.equals(i1));
        Test.println(i1 == i1);

        Test.println(i1.equals(i2));
        Test.println(i1 == i2);

        Test.println(i1.equals(i3));
        Test.println(i1 == i3);

        Test.println(i1.equals(new Integer(123)));
        Test.println(i1 == new Integer(123));
    }
}
