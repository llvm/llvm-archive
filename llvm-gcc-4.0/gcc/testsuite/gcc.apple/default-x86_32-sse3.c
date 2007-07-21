/* APPLE LOCAL file 4515157 */
/* { dg-do compile { target i?86-*-darwin* } } */
/* { dg-options "-m32" } */
/* Insure that -m32 (absence of -m64) does not imply -msse3 on Darwin/x86.  */
#include <stdlib.h>
#ifdef __SSE3__
#error "SSE3 should be disabled by default for x86 -m32"
#endif
main ()
{
  exit (0);
}
