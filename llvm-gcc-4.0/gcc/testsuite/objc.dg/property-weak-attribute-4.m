/* APPLE LOCAL file radar 4660366 */
/* Test that we call objc_assign_weak/objc_read_weak in 'weak' property 
   accessors without specifying -fobjc-gc.  */
/* { dg-do compile { target *-*-darwin* } } */

@interface TestWeak
@property(ivar, weak) id object;
@end

@implementation TestWeak
@end
/* { dg-final { scan-assembler "objc_assign_weak" } } */
/* { dg-final { scan-assembler "objc_read_weak" } } */
