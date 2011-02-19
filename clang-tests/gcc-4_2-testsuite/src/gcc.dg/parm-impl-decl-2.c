/* Test implicit function declarations and other odd declarations in
   function prototypes.  Make sure that LABEL_DECLs don't occur.  */
/* Origin: Joseph Myers <joseph@codesourcery.com> */
/* { dg-do compile } */
/* { dg-options "" } */

int k (int [sizeof &&z]); /* { dg-error "error: use of address-of-label extension outside of a function" } */
