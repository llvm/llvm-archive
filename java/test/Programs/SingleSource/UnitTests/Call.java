public class Call
{
    interface Interface {
        public int i1();
    }

    private static abstract class Abstract {
        protected abstract int a1();
        public int c1() { return a1() * a2(); }
        protected abstract int a2();
    }

    private static class Concrete extends Abstract implements Interface {
        private int i;

        Concrete(int i) { this.i = i; }
        public int a1() { return i; }
        public int a2() { return i + i; }
        public int i1() { return c1(); }
    }

    public static void main(String[] args) {
        Interface i = new Concrete(10);
        Test.print_int_ln(i.i1());
    }
}
