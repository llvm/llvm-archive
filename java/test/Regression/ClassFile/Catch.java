public class Catch
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

        return i;
    }
}
