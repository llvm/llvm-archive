/* Test diagnostics for bad type conversion when inlining unprototyped
   functions: should not be errors with -pedantic-errors.  */
/* Origin: Joseph Myers <jsm@polyomino.org.uk> */
/* { dg-do compile } */
/* { dg-options "-O3 -std=c99 -pedantic-errors" } */

/* This is valid to execute, so maybe shouldn't warn at all.  */
void f0(x) signed char *x; { }
void g0(unsigned char *x) { f0(x); } /* { dg-error "passing 'unsigned char \\*' to parameter of type 'signed char \\*'" } */

/* This is undefined on execution but still must compile.  */
void f1(x) int *x; { }
void g1(unsigned int *x) { f1(x); } /* { dg-error "passing 'unsigned int \\*' to parameter of type 'int \\*'" } */
