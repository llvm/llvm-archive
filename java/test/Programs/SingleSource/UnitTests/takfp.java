public class takfp {
    public static void main(String args[]) {
        int n = 10;
        Test.print_double_ln( Tak(n*3.0f, n*2.0f, n*1.0f) );
    }

  public static float Tak (float x, float y, float z) {
    if (y >= x) return z;
    else return Tak(Tak(x-1.0f,y,z), Tak(y-1.0f,z,x), Tak(z-1.0f,x,y));
  }
}