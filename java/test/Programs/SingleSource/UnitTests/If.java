public class If
{
    private static boolean isZero(int i) {
        return i == 0;
    }

    public static void main(String[] args) {
        int i = 0;
        if (isZero(i))
            Test.print_int_ln(0);
        else
            Test.print_int_ln(1);
    }
}
