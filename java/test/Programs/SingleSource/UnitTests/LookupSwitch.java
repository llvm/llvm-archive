public class LookupSwitch
{
    public static void main(String[] args) {
        switch (128) {
        case 0: Test.println(255);
        case 128: Test.println(128);
        case 255: Test.println(0);
        default: Test.println(-1);
        }
    }
}
