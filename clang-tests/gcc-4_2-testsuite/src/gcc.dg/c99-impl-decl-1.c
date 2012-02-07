/* Test for implicit function declaration: in C90 only.  */
/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do compile } */
/* { dg-options "-std=iso9899:1999 -pedantic-errors" } */

void
foo (void)
{
  bar (); /* { dg-error "implicit declaration of function 'bar' is invalid in C99" } */
}

/* C90 subclause 7.1.7 says we can implicitly declare strcmp; C99 removes
   implict declarations.
*/
int
bar (const char *a, const char *b)
{
  return strcmp (a, b); /* { dg-error "implicitly declaring library function .strcmp" } */
                        /* { dg-warning "please include the header" "" { target *-*-* } 18 } */
}
