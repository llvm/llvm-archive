public class nestedloop {
    public static void main(String args[]) {
        int n = 18;
        int x = 0;
        for (int a=0; a<n; a++)
            for (int b=0; b<n; b++)
                for (int c=0; c<n; c++)
                    for (int d=0; d<n; d++)
                        for (int e=0; e<n; e++)
                            for (int f=0; f<n; f++)
                                x++;
        Test.println(x);
    }
}
