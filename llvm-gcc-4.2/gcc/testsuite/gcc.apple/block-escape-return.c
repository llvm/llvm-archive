/* APPLE LOCAL file radar 6083129 byref escapes */
/* { dg-options "-fblocks" } */
/* { dg-do run } */


#include <stdio.h>

extern void abort(void);
void _Block_byref_assign_copy(void *a, void *b){};

static int count;
static void _Block_byref_release(void * arg) {
  printf ("%p\n", arg);
  ++count;
}


int main() {
  __byref  int X = 1234;
  if (X) {
    __byref int local_BYREF = 100;
    X += 100 + local_BYREF;
    return count-2;
  }
  return -1;
}
