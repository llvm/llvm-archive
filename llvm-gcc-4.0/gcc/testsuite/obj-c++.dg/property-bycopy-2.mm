/* APPLE LOCAL file radar 4625843 */
/* Test that bycopy calls are generated. */
/* { dg-do compile { target *-*-darwin* } } */
@protocol NSCopying;

@interface NSWindow 
{
	NSWindow* IVAR;
}
@end

@implementation NSWindow @end

@interface NSWindow (CAT)
@property(ivar=IVAR, bycopy) NSWindow <NSCopying>* title;
@end

@implementation NSWindow (CAT)
@property(ivar=IVAR, bycopy) NSWindow <NSCopying>* title;
@end
/* { dg-final { scan-assembler "object_getProperty_bycopy" } } */
/* { dg-final { scan-assembler "object_setProperty_bycopy" } } */
