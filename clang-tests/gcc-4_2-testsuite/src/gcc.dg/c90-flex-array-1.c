/* Test for flexible array members.  Test for rejection in C90 mode.  */
/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do compile } */
/* { dg-options "-std=iso9899:1990 -pedantic-errors" } */

struct flex { int a; int b[]; }; /* { dg-bogus "warning" "warning in place of error" } */
/* { dg-error "Flexible array members are a C99-specific feature" "" { target *-*-* } 6 } */
