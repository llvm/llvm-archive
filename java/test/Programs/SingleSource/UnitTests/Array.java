public class Array
{
    public static void main(String[] args) {
        int[] intArray = new int[10];

        for (int i = 0, e = intArray.length; i != e; ++i)
            intArray[i] = intArray.length - i;

        Util.printlnElements(intArray);


        Object[] objectArray = new Object[10];

        for (int i = 0, e = objectArray.length; i != e; ++i)
            objectArray[i] = intArray;

        intArray = (int[]) objectArray[4];
        Util.printlnElements(intArray);
    }
}
