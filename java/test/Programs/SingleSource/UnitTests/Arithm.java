package edu.uiuc.cs.llvm;

public class Arithm
{
    public static int main(String[] args) {
        int one = 1;
        int two = 2;

        return (one + two) - (two * two) + (two / one) + (two % one) + (two << one) - (two >> 1) + (-two); // = 2
    }
}
