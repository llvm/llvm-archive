/* { dg-do compile } */
/* { dg-options "-O2 -Wno-integer-overflow" } */

#include <limits.h>

int foo = INT_MAX + 1;

