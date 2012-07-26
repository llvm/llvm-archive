/* APPLE LOCAL file radar 6252174 */
/* { dg-options "-m64 -mmacosx-version-min=10.6" } */
/* { dg-do compile { target *-*-darwin* } } */

__attribute__((objc_root_class)) @interface NSObject
-method;
@end

@interface Sub : NSObject @end

@implementation Sub
-method { [super method]; }
@end
/* { dg-final { scan-assembler-not ".section __DATA, __objc_msgrefs, coalesced\n\t.align 4\n\tl_objc_msgSendSuper2_fixup_method" } } */
