/* APPLE LOCAL file 5932809 */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

#include <stdio.h>
void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}
extern void exit(int);

typedef double (^myblock)(int);


double test(myblock I) {
  return I(42);
}

int main() {
  __byref int x = 1;
  __byref int y = 2;
  double res = test(^(int z){|y| y = x+z; return (double)x; }); /* { dg-warning "has been deprecated in blocks" } */
  printf("result = %f  x = %d y = %d\n", res, x, y);
  if (x != 1 || y != 43)
   exit(1);

  res = test(^(int z){|x| x = x+z; return (double)y; }); /* { dg-warning "has been deprecated in blocks" } */
  printf("result = %f  x = %d y = %d\n", res, x, y);
  if (x != 43 || y != 43)
    exit(1);
  return 0;
}

