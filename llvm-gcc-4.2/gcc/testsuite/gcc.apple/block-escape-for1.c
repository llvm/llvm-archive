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
  __block int O1;
  int i;
  int p;
  for (i = 1; i <= 5; i++) {
    __block int I1;
    p = 0;
    while (p != 10) {
      __block int II1;
      if (p == 2)
	break;
      ++p;
    }
  }
 }
  return count-21;
}
