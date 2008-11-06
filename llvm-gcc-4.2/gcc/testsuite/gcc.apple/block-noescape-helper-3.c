/* APPLE LOCAL file radar 6083129 byref escapes */
/* Test for generation of escape _Block_byref_release call when a __block
   variable inside a block is declared and used. */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

#include <stdio.h>

void *_NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}

extern void abort(void);

static int count;
static void _Block_byref_release(void * arg) {
        ++count;
}

void junk(void (^block)(void)) {
  block();
}

int test() {
  void (^dummy)(void) = ^{ int __block i = 10; printf("i = %d\n", i); };
  junk(dummy);
  return count;
}

int main()
{
	if ( test() != 1)
	  abort();
	return 0;
}

