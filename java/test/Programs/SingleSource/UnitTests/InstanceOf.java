class InstanceOfBase
{
    int foo() { return 0; }
    int bar() { return 0; }
}

interface InstanceOfInterface {
    public int baz();
}

public class InstanceOf extends InstanceOfBase implements InstanceOfInterface
{
    int foo() { return 1; }
    public int baz() { return foo(); }

    public static void main(String[] args) {
        InstanceOfBase aa = new InstanceOfBase();
        InstanceOfBase ab = new InstanceOf();
        InstanceOfInterface bb = new InstanceOf();

        boolean aaA = aa instanceof InstanceOfBase;
        boolean aaB = aa instanceof InstanceOf;
        boolean aaI = aa instanceof InstanceOfInterface;

        boolean abA = ab instanceof InstanceOfBase;
        boolean abB = ab instanceof InstanceOf;
        boolean abI = ab instanceof InstanceOfInterface;

        boolean bbA = bb instanceof InstanceOfBase;
        boolean bbB = bb instanceof InstanceOf;
        boolean bbI = bb instanceof InstanceOfInterface;
    }
}
