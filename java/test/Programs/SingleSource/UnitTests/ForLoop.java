public class ForLoop
{
    public static void main(String[] args) {
        int sum = 0;
        for (int i = 0; i < 100; ++i)
            sum += i;

        Test.println(sum);
    }
}
