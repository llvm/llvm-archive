public class PrivateCall
{
    private int a;

    public PrivateCall(int i) {
        a = i;
    }

    private int foo(int i) {
        return i * i / a;
    }

    public static void main(String[] args) {
        PrivateCall p = new PrivateCall(7);
        Test.println(p.foo(123));
    }
}
