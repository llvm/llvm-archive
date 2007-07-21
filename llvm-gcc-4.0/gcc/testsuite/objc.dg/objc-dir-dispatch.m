/* APPLE LOCAL file radar 4590221 */
/* Check that -fobjc-direct-dispatch is on by default for 32-bit ppc with 
   -mmacosx-version-min=10.4. */
/* { dg-do compile } */
/* { dg-options "-mmacosx-version-min=10.4" } */
/* { dg-skip-if "" { powerpc*-*-darwin* } { "-m64" } { "" } } */

#include <objc/Object.h>

@interface Derived: Object {
@public
  Object *other;
}
@end

void foo(void) {
  Derived *o = [Derived new];
  o->other = 0;  
}

/* { dg-final { scan-assembler "objc_msgSend" { target i?86-*-* } } } */

/* { dg-final { scan-assembler "bla.*fffeff00" { target powerpc*-*-darwin* } } } */
