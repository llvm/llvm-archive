public class Strings
{
    public static void main(String[] args) {
        String foo = "foo";
        String bar = "bar";
        String foobar = "foobar";

        Test.print_boolean_ln(foobar == foo + bar);
        Test.print_boolean_ln(foobar.equals(foo + bar));
    }
}
