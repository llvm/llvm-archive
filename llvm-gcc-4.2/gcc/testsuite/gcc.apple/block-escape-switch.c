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

int foo(int p, int q) {
  __block int O1;
  switch (p) {
  case 1:
    {
      __block int I1;
      I1 += 1;
      break;
    }
  case 10:
    {
      __block int J1;
      break;
    }
  default :
    {
      __block int D1;
      __block int D2;
      switch (q) {
      case 11:
	{
	  __block int  Q1;
	  break;
	}
      default:
	{
	  __block int  ID1;
	  __block int  ID2;
	}
      };
      break;
    }
  }
  return 0;
}

int main() {
  foo (1, 0);
  if (count != 2)
    abort();

  count = 0;
  foo (12, 11);
  if (count != 4)
    abort();

  count = 0;
  foo (12, 13);
  if (count != 5)
    abort();

  return 0;
}
