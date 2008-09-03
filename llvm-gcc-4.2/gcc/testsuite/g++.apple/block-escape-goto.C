/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do compile } */

int main(int p) {
  p = -5;
  __byref int O1;
  int i;
 LOUT: ;
  for (i = 1; i < 100; i++) {
    __byref int I1;
    while (p < 0) {
      __byref int II1;	
      if (p == 100)
	goto LOUT;
      ++p;
      if (p == 2345)
	break;
    }
  }
  return 0;
}
