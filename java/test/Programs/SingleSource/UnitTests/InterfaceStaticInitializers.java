public class InterfaceStaticInitializers
{
    public static class I1 {
        public static int i1 = 1;
        public static int i2 = 2;
    }

    public static void main(String[] args) {
        Test.println(I1.i1);
        Test.println(I1.i2);
    }
}
