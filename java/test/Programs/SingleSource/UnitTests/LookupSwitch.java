public class LookupSwitch
{
    public static void main(String[] args) {
        switch (128) {
        case 0: System.out.println(255);
        case 128: System.out.println(128);
        case 255: System.out.println(0);
        default: System.out.println(-1);
        }
    }
}
