public class FieldAccess
{
    private static class Base {
        int i = 1;
        float f = 1.0F;
    }

    private static class Derived extends Base {
        int i = -1;
        double d = -1.0;
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
