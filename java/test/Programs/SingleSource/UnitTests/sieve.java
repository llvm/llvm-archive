public class sieve {
    public static void main(String args[]) {
        int NUM = 1200;
        boolean [] flags = new boolean[8192 + 1];
        int count = 0;
        while (NUM-- > 0) {
            count = 0;
            for (int i=2; i <= 8192; i++) {
                flags[i] = true;
            }
            for (int i=2; i <= 8192; i++) {
                if (flags[i]) {
                    // remove all multiples of prime: i
                    for (int k=i+i; k <= 8192; k+=i) {
                        flags[k] = false;
                    }
                    count++;
                }
            }
        }
        Test.println(count);
    }
}
