public class TableSwitch
{
    public static void main(String[] args) {
        switch (4) {
        case 0: Test.println(4);
        case 1: Test.println(3);
        case 2: Test.println(2);
        case 3: Test.println(1);
        case 4: Test.println(0);
        default: Test.println(-1);
        }
    }
}
