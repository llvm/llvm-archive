public class FloatCompare
{
    private static float[] doubles =
        new float[]{ -1.0f, 0.0f, -0.0f, 1.0f,
                     Float.NaN,
                     Float.NEGATIVE_INFINITY,
                     Float.POSITIVE_INFINITY };

    public static void main(String[] args) {
        int greater = 0;
        int equal = 0;
        int less = 0;
        int unordered = 0;

        for (int i = 0; i < doubles.length; ++i) {
            double a = doubles[i];
            for (int j = 0; j < doubles.length; ++j) {
                double b = doubles[j];
                if (a > b)
                    ++greater;
                else if (a < b)
                    ++less;
                else if (a == b)
                    ++equal;
                else
                    ++unordered;
            }
        }

        Test.println(greater);
        Test.println(equal);
        Test.println(less);
        Test.println(unordered);
    }
}
