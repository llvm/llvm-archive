public class TableSwitch
{
    public static void main(String[] args) {
        switch (4) {
        case 0: Test.print_int_ln(4);
        case 1: Test.print_int_ln(3);
        case 2: Test.print_int_ln(2);
        case 3: Test.print_int_ln(1);
        case 4: Test.print_int_ln(0);
        default: Test.print_int_ln(-1);
        }
    }
}
