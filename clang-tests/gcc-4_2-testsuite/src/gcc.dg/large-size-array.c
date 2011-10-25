/* { dg-do compile } */
#include <limits.h>

#ifdef __LP64__
#define DIM UINT_MAX>>1
/* { dg-warning "expanded from" "" { target *-*-* } 5 } */
#else
#define DIM USHRT_MAX>>1
#endif

int
sub (int *a)
{
  return a[0];
}

int
main (void)
{
  int a[DIM][DIM];  /* { dg-error "array is too large" } */
  return sub (&a[0][0]);
}
/* { dg-warning "expanded from" "" { target *-*-* } 73 } */
