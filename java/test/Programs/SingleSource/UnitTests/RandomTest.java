import java.util.*;

public class RandomTest
{
    public static Random rand = new Random(0);

    public static void main(String[] args) {
        for (int i = 0; i < 1000; ++i)
            Test.println(rand.nextInt());
    }
}
