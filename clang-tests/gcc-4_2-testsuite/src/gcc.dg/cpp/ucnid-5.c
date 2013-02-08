/* { dg-do compile } */
/* { dg-options "-std=c99 -pedantic" } */

int \u00AA;
int \u00AB; /* { dg-error "expected identifier" } */
int \u00B6; /* { dg-error "expected identifier" } */
int \u00BA;
int \u00C0;
int \u00D6;
int \u0384; /* { dg-error "expected identifier" } */

int \u0669; /* { dg-error "expected identifier" } */
int A\u0669;

/* APPLE LOCAL <rdar://problem/13134700> */
/* These are valid preprocessor numeric tokens, but invalid parsing tokens.
   Clang will accept the former, but there's no way to tell if it's made up
   of one token or two. */
#define TOKEN1 0\u00BA; 
#define TOKEN2 0\u0669;

int \u0E59; /* { dg-error "expected identifier" } */
int A\u0E59;
