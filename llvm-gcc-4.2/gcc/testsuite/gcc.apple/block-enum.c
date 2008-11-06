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
  __block enum numbers x = one;
  __block enum numbers y = two;

  enum numbers res = test(^(enum numbers z){y = z; return x; }); 

  if (x != one || y != three || res != one)
   exit(1);

  res = test(^(enum numbers z){x = z; return x; }); 
  if (x != three || res != three)
    exit(1);
  return 0;
}
