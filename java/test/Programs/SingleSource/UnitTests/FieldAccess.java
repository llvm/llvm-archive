class A {
    int i;
    float f;
}

class B extends A {
    int i;
    double d;
}

public class FieldAccess
{
    public static void main(String[] args) {
        B b = null;
        b.d = 4.0;
        b.i = 3;
        b.f = 2.0F;
        ((A) b).i = 4;
        ((A) b).f = 1.0F;
    }
}
