public class Arithm
{
    public static void main(String[] args) {
        int i = 53;

        i += 53;
        i *= 53;
        i -= 53;
        i >>= 3;
        i /= 53;
        i %= 53;
        i = -i;
        i <<= 3;
        i |= 53;
        i ^= 35;
        i &= 53;
        Test.println(i);
    }
}
