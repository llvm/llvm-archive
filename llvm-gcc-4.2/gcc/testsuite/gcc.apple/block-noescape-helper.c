/* APPLE LOCAL file radar 6083129 byref escapes */
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
  int __byref i = 10;
  void (^dummy)(void) = ^{ | i | ++i; };	/* { dg-warning "has been deprecated in blocks" } */
  junk(dummy);
  return count;
}

int main()
{
	if ( test() != 1)
	  abort();
	return 0;
}

