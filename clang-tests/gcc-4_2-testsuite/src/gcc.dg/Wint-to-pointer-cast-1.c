/* Test -Wint-to-pointer-cast - on by default.  */
/* Origin: Joseph Myers <joseph@codesourcery.com> */
/* { dg-do compile } */
/* { dg-options "" } */

char c;

void *
f (void)
{
  return (void *) c; /* { dg-warning "from smaller integer type" } */
}
