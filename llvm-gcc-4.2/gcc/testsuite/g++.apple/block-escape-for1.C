/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


#include <stdio.h>

extern "C" void abort(void);

static int count;
static void _Block_byref_release(void * arg) {
  printf ("%p\n", arg);
  ++count;
}

int main() {
  {
    __byref int O1;
    int i;
    int p;
    for (i = 1; i <= 5; i++) {
      __byref int I1;
      p = 0;
      while (p != 10) {
	__byref int II1;
	if (p == 2)
	  break;
	++p;
      }
    }
  }
  return count-21;
}
