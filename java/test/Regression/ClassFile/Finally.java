public class Finally
{
    public static int main(String[] args) {
        int i = 0;

        try {
            i += 10;
        }
        catch (Exception e) {
            System.err.println("caught exception: " + e);
            i += 20;
        }
        finally {
            System.err.println("executing finally clause");
            i += 30;
        }

        return 0;
    }
}
