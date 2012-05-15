/* Test attempt to redefine 'id' in an incompatible fashion.  */
/* { dg-do compile } */

typedef int id;  /* { dg-error "typedef redefinition with different types" } */

id b;
