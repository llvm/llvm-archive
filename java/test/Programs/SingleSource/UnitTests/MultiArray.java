public class MultiArray
{
    public static void main(String[] args) {
        int[][] intArray = new int[10][10];

        for (int i = 0; i != intArray.length; ++i)
            for (int j = 0; j != intArray[i].length; ++j)
                intArray[i][j] = (j+1) * (i+1);

        for (int i = 0; i != intArray.length; ++i)
            for (int j = 0; j != intArray[i].length; ++j)
                Test.println(intArray[i][j]);
    }
}
