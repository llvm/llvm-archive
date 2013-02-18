/* Test for non-lvalue arrays: test that the unary '&' operator is not
   allowed on them, for both C90 and C99.  */

/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do compile } */
/* { dg-options "-std=iso9899:1990 -pedantic-errors" } */

struct s { char c[1]; };

extern struct s foo (void);
struct s a, b, c;
int d;

void
bar (void)
{
  &((foo ()).c); /* { dg-bogus "warning" "warning in place of error" } */ 
  &((d ? b : c).c); /* { dg-bogus "warning" "warning in place of error" } */
  &((d, b).c); /* { dg-bogus "warning" "warning in place of error" } */
  &((a = b).c); /* { dg-bogus "warning" "warning in place of error" } */
}

/* { dg-error "cannot take the address of an rvalue" "" { target *-*-* } 17 } */
/* { dg-error "cannot take the address of an rvalue" "" { target *-*-* } 18 } */
/* { dg-error "cannot take the address of an rvalue" "" { target *-*-* } 19 } */
/* { dg-error "cannot take the address of an rvalue" "" { target *-*-* } 20 } */
