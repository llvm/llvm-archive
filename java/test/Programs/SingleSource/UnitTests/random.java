public class random {

    public static final int IM = 139968;
    public static final int IA = 3877;
    public static final int IC = 29573;

    public static void main(String args[]) {
        int N = 900000;

        while (--N > 0) {
            gen_random(100);
        }
        Test.println(gen_random(100));
    }

    public static int last = 42;
    public static double gen_random(double max) {
        return( max * (last = (last * IA + IC) % IM) / IM );
    }
}
