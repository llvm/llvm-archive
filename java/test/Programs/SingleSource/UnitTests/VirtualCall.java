public class VirtualCall
{
    static class A
    {
        public void m1() { Test.println(1); m2(); m3(); }
        public void m2() { Test.println(2); m3(); }
        public void m3() { Test.println(3); }
    }

    static class B extends A
    {
        public void m2() { Test.println(22); }
        public void m3() { Test.println(33); }
    }

    static class C extends B
    {
        public void m2() { Test.println(222); m3(); }
    }

    public static void main(String[] args) {
        A a = new A();
        a.m1();
        a.m2();
        a.m3();

        a = new B();
        a.m1();
        a.m2();
        a.m3();

        a = new C();
        a.m1();
        a.m2();
        a.m3();
    }
}
