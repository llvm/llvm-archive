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

void objc_get_type_qualifiers (int flag, int type) {
  while (flag--)
    while (type++ < 4) {
      __byref int W1;
      __byref int W2;
      if (type == 2)
	break;
    }
}

int main() {
  objc_get_type_qualifiers (1, 0);
  if (count != 4)
    abort();
  return 0;
}
