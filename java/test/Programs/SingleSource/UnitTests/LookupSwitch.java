public class LookupSwitch
{
    public static void main(String[] args) {
        switch (128) {
        case 0: Test.print_int_ln(255);
        case 128: Test.print_int_ln(128);
        case 255: Test.print_int_ln(0);
        default: Test.print_int_ln(-1);
        }
    }
}
