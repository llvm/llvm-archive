/* APPLE LOCAL file 5435676 */
/* Test that message-to-super case now uses a different section __super_refs */
/* { dg-options "-fobjc-abi-version=2" } */
/* { dg-do compile { target *-*-darwin* } } */

@interface Super { id isa; } @end

@implementation Super
	+method { return self; } 
@end

@interface Sub : Super @end

@implementation Sub
	+method { return [super method]; }
@end

int main() {
    [Sub method];
}
/* { dg-final { scan-assembler "__super_refs" } } */
/* { dg-final { scan-assembler "OBJC_CLASSLIST_SUP_REFS_\\\$_0:" } } */
