public class AbstractCall
{
    private static abstract class Abstract {
        abstract int abstractMethod();

        int concreteMethod() { return abstractMethod(); }
    }

    private static class Concrete extends Abstract {
        int abstractMethod() { return 5; }
    }

    public static void main(String[] args) {
        Abstract a = new Concrete();
        Test.println(a.concreteMethod());
    }
}
