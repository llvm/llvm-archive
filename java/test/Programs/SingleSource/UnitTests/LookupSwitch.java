package edu.uiuc.cs.llvm;

public class LookupSwitch
{
    public static int main(String[] args) {
        switch (128) {
        case 0: return 255;
        case 128: return 128;
        case 255: return 0;
        default: return -1;
        }
    }
}
