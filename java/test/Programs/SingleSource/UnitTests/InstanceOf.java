class A
{
    int foo() { return 0; }
    int bar() { return 0; }
}

interface I {
    public int baz();
}

class B extends A implements I
{
    int foo() { return 1; }
    public int baz() { return foo(); }
}

public class InstanceOf
{
    public static void main(String[] args) {
        A aa = new A();
        A ab = new B();
        B bb = new B();

        boolean aaA = aa instanceof A;
        boolean aaB = aa instanceof B;
        boolean aaI = aa instanceof I;

        boolean abA = ab instanceof A;
        boolean abB = ab instanceof B;
        boolean abI = ab instanceof I;

        boolean bbA = bb instanceof A;
        boolean bbB = bb instanceof B;
        boolean bbI = bb instanceof I;
    }
}
