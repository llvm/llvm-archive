/* Test for overflow in NULL pointer constant.  */
/* { dg-do compile } */

#include <limits.h>

void *p = 0;

void *q = 0 * (INT_MAX + 1);  // { dg-warning "expression which evaluates to zero treated as a null pointer constant" }


