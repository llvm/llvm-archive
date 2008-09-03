/* APPLE LOCAL file radar 5957801 */
/* Test for use of block pointer in a ?-exp expression. */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

typedef int (^myblock)(void);
void *_NSConcreteStackBlock;

myblock foo(int i, myblock b) {
  if (!i ? (void *)0 : b)
    return (i ? b : (void *)0);
}

int main () {
  myblock b = ^{ return 1; };
  if (foo (1, b))
    return 0;
  return 1;
}
