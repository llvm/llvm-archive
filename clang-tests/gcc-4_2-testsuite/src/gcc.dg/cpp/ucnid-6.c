/* { dg-do compile } */
/* { dg-options "-std=c89" } */
#define a b(
#define b(x) q
int a\u00aa); /* { dg-warning "universal character names are only valid in C99 or C\\+\\+" } */
