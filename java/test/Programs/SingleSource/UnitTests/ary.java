public class ary {
    public static void main(String args[]) {
        int i, j, k, n = 9000;
        int x[] = new int[n];
        int y[] = new int[n];

        for (i = 0; i < n; i++)
            x[i] = i + 1;
        for (k = 0; k < 1000; k++ )
            for (j = n-1; j >= 0; j--)
                y[j] += x[j];

        Test.print_int_ln(y[0]);
        Test.print_int_ln(y[n-1]);
    }
}