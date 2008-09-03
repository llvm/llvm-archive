/* APPLE LOCAL file radar 5732232 - blocks */
/* Test use of enumerators in blocks. */
/* { dg-do run } */
/* { dg-options "-fblocks" } */

#include <stdio.h>
void * _NSConcreteStackBlock;
extern void exit(int);

enum numbers
{
    zero, one, two, three
};

typedef enum numbers (^myblock)(enum numbers);


double test(myblock I) {
  return I(three);
}

int main() {
  enum numbers x = one;
  enum numbers y = two;

  enum numbers res = test(^(enum numbers z){|y| y = z; return x; }); /* { dg-warning "has been deprecated in blocks" } */

  if (x != one || y != three || res != one)
   exit(1);

  res = test(^(enum numbers z){|x| x = z; return x; }); /* { dg-warning "has been deprecated in blocks" } */
  if (x != three || res != three)
    exit(1);
  return 0;
}
