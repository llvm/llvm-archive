/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

#include <stdio.h>

extern void abort(void);

static int count;
static void _Block_byref_release(void * arg) {
  printf ("%p\n", arg);
  ++count;
}

int main() {
  {
    __byref int O1;
    int p = 0;
    do {
      __byref int I1;
      do {
	__byref int J1;
	if (p == 2)
	  break;
      } while ( ++p < 3);
      if (p == 4)
	break;
    } while (++p != 5);
  }

  return count-7;
}
