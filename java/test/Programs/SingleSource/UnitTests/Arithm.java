public class Arithm
{
    public static void main(String[] args) {
        int one = 1;
        int two = 2;

        Test.println((one + two) - (two * two) + (two / one) + (two % one) + (two << one) - (two >> 1) + (-two)); // = 2
    }
}
