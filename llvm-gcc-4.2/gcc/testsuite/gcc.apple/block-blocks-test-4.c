/* APPLE LOCAL file 5932809 */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

#include <stdio.h>
void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}

int main() {
   __byref  int X = 1234;

   int (^CP)(void) = ^{ |X| X = X+1;  return X; }; /* { dg-warning "has been deprecated in blocks" } */
   CP();
   printf ("X = %d\n", X);
   return X - 1235;
}
