/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


#include <stdio.h>

extern "C" void abort(void);
void _Block_byref_assign_copy(void *a, void *b){};

static int count;
static void _Block_byref_release(void * arg) {
  printf ("%p\n", arg);
  ++count;
}

void *_NSConcreteStackBlock;

void FOO(int arg) {
  __byref int X = 1234;
  if (arg) {
    __byref int local_BYREF = 100;
    X += 100 + local_BYREF;
    return;
  }
  ^{ X++; };
  X = 1000;
}

int main() {
  FOO(1);
  if (count != 2)
    abort();

  count = 0;
  FOO(0);
  if (count != 1)
    abort();
  return 0;
}
