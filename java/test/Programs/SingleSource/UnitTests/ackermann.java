public class ackermann {
    public static void main(String[] args) {
        Test.println(Ack(3, 10));
    }
    public static int Ack(int m, int n) {
        return (m == 0) ? (n + 1) : ((n == 0) ? Ack(m-1, 1) :
                                     Ack(m-1, Ack(m, n - 1)));
    }
}
