public class InnerClass
{
    private int i = 1024;
    private boolean b = true;

    public int i1() { return i; }
    public boolean b1() { return b; }

    InnerClassInner getInner() {
        return new InnerClassInner();
    }

    public class InnerClassInner {
        private int i = 512;
        private boolean b = false;

        public int ii1() { return i1(); }
        public int ii2() { return i; }
        public boolean bb1() { return b1(); }
        public boolean bb2() { return b; }
    }

    public static void main(String[] args) {
        InnerClass i = new InnerClass();
        Test.println(i.i1());
        Test.println(i.b1());

        InnerClassInner ii = i.getInner();
        Test.println(ii.ii1());
        Test.println(ii.bb1());
        Test.println(ii.ii2());
        Test.println(ii.bb2());
    }
}
