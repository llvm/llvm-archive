public class FieldAccess
{
    private static class Base {
        int i;
        float f;
    }

    private static class Derived extends Base {
        int i;
        double d;
    }

    public static void main(String[] args) {
        Derived b = new Derived();
        b.d = 4.0;
        b.i = 3;
        b.f = 2.0F;
        ((Base) b).i = 4;
        ((Base) b).f = 1.0F;

        Test.println(((Base)b).i);
        Test.println(b.i);
        Test.println(((Base)b).f);
        Test.println(b.f);
        Test.println(b.d);
    }
}
