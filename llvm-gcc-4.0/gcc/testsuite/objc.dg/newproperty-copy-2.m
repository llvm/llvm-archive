/* APPLE LOCAL file radar 4805321 */
/* Test that bycopy calls are generated. */
/* { dg-options "-fobjc-new-property" } */
/* { dg-do compile { target *-*-darwin* } } */
@protocol NSCopying;

@interface NSWindow 
{
	NSWindow* IVAR;
}
@property(copy) NSWindow <NSCopying>* title;
@end

@implementation NSWindow 
@dynamic title;
@end

@interface NSWindow (CAT)
@property(copy) NSWindow <NSCopying>* title;
@end

@implementation NSWindow (CAT)
@synthesize title=IVAR;
@end
/* { dg-final { scan-assembler "object_getProperty_bycopy" } } */
/* { dg-final { scan-assembler "object_setProperty_bycopy" } } */
