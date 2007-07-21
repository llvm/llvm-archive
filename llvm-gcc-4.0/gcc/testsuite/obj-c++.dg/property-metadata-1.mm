/* APPLE LOCAL file radar 4498373 */
/* Test for a Synthesized Property */
/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-fobjc-abi-version=2" } */
/* { dg-skip-if "" { powerpc*-*-darwin* } { "-m64" } { "" } } */

#include <objc/Object.h>

@interface Number : Object
@property (ivar) double value;
@end

@implementation Number
@property double value;
@end
/* { dg-final { scan-assembler ".long\t8\n\t.long\t1\n\t.long\t.*\n\t.long\t.*" } } */
