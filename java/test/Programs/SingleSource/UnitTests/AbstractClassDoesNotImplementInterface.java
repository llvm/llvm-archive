public class AbstractClassDoesNotImplementInterface {

    private static interface Interface {
        public void iMethod();
    }

    private static abstract class Abstract implements Interface {

    }

    private static class Concrete extends Abstract {
        public void iMethod() { Test.println(12345); }
    }

    public static void main(String[] args) {
        Interface i = new Concrete();
        i.iMethod();
    }
}
