public class If
{
    private static boolean isZero(int i) {
        return i == 0;
    }

    public static void main(String[] args) {
        int i = 0;
        if (isZero(i))
            Test.println(0);
        else
            Test.println(1);
    }
}
