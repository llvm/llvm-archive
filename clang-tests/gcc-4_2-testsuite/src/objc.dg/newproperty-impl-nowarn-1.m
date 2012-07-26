/* APPLE LOCAL file radar 4816280 */
/* No warning for implementation with user-accessor, @dynamic or @synthesize properties. */
/* { dg-options "-fobjc-new-property -mmacosx-version-min=10.5" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-options "-fobjc-new-property" { target arm*-*-darwin* } } */
/* { dg-do compile { target *-*-darwin* } } */

__attribute__((objc_root_class)) @interface Larry
@property int ivar;
@end

@implementation Larry
// no warning
- (void)setIvar:(int)arg{};
- (int)ivar{};
@end

__attribute__((objc_root_class)) @interface Moe
{
    int ivar;
}
@property int ivar;
@end

@implementation Moe
// no warning
@synthesize ivar;
- (void)setIvar:(int)arg{};
@end

__attribute__((objc_root_class)) @interface Curly
@property int ivar;
@end

@implementation Curly
// no warning
@dynamic ivar;
- (int)ivar{};
@end

int main (void) {return 0;}

