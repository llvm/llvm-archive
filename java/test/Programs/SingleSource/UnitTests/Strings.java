public class Strings
{
    public static void main(String[] args) {
        String foo = "foo";
        String bar = "bar";
        String foobar = "foobar";

        Test.println(foobar == foo + bar);
        Test.println(foobar.equals(foo + bar));
    }
}
