public class LongCompare
{
    public static void main(String[] args) {
        long l1 = 123456789123456789L;
        long l2 = 987654321987654321L;

        Test.print_long_ln(l1);
        Test.print_long_ln(l2);
        Test.print_boolean_ln(l1 == l2);
    }
}
