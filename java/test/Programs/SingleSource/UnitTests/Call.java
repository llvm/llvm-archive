interface CallInterface {
    public int i1();
}

abstract class CallAbstract {
    protected abstract int a1();
    public int c1() { return a1() * a2(); }
    protected abstract int a2();
}

class CallConcrete extends CallAbstract implements CallInterface {
    private int i;

    CallConcrete(int i) { this.i = i; }
    public int a1() { return i; }
    public int a2() { return i + i; }
    public int i1() { return c1(); }
}

public class Call
{
    public static void main(String[] args) {
        CallInterface i = new CallConcrete(10);
        Test.print_int_ln(i.i1());
    }
}
