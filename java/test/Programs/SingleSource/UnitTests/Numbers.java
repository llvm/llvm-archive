public class Numbers
{
    public static void main(String[] args) {
        Number[] numbers = new Number[4];

        numbers[0] = new Byte((byte)123);
        numbers[1] = new Integer(1234567890);
        numbers[2] = new Long(1234567890987654321L);
        numbers[3] = new Short((short)12345);

        for (int i = 0; i != numbers.length; ++i) {
            Number n = numbers[i];
            Test.print_long_ln(n.longValue());
            Test.print_double_ln(n.doubleValue());
         }
    }
}
