public class AbstractClassDoesNotImplementInterface {

    private static interface Interface {
        public void iMethod();
    }

    private static abstract class Abstract implements Interface {

    }

    private static class Concrete extends Abstract {
        public void iMethod() { }
    }

    public static void main(String[] args) {
        Abstract a = new Concrete();
    }
}
