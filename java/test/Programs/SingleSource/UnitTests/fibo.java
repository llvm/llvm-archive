public class fibo {
    public static void main(String args[]) {
        int N = 32;
        Test.print_int_ln(fib(N));
    }
    public static int fib(int n) {
        if (n < 2) return(1);
        return( fib(n-2) + fib(n-1) );
    }
}