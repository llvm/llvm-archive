class A
{
    int foo() { return 0; }
    int bar() { return 0; }
}

class B extends A
{
    int foo() { return 1; }
}

public class VTable
{
    public static void main(String[] args) {
        A a = new A();
        a.foo();
        a.bar();

        a = new B();
        a.foo();
        a.bar();
    }
}
