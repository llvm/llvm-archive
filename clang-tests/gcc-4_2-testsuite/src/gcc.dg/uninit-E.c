/* Test we do warn about initializing variable with self when -Winit-self is supplied. */
/* { dg-do compile } */
/* { dg-options "-O -Wuninitialized -Winit-self" } */

int f()
{
  int i = i; /* { dg-error "" } */
  return i; /* { dg-warning "variable 'i' is uninitialized when used here" }  */
}
