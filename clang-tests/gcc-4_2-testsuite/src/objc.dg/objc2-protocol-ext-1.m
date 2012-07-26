/* APPLE LOCAL file radar 5192466 - radar 10492418 */
/* Check the size of protocol meta-data. */
/* { dg-options "-mmacosx-version-min=10.5 -m64" } */
/* { dg-do compile { target powerpc*-*-darwin* i?86*-*-darwin* } } */

@protocol Proto1
@end

@protocol Proto2
@end

__attribute__((objc_root_class)) @interface Super <Proto1, Proto2> { id isa; } @end
@implementation Super @end
/* { dg-final { scan-assembler ".long\t80\n\t.long\t0" } } */
