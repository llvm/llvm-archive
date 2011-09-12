/* Origin: PR c/179 from Gray Watson <gray@256.com>, adapted as a testcase
   by Joseph Myers <jsm28@cam.ac.uk>.  */
/* { dg-do compile } */
/* { dg-options "-O2 -Wuninitialized" } */
extern void foo (int *);
extern void bar (int);

void
baz (void)
{
  int i; /* { dg-error "variable" } */
  if (i) /* { dg-warning "variable 'i' is uninitialized when used here" } */
    bar (i);
  foo (&i);
}
