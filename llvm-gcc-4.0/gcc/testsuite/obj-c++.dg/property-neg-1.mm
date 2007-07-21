/* APPLE LOCAL file radar 4436866 */
/* This program checks for proper use of 'readonly' attribute. */
/* { dg-do compile { target *-*-darwin* } } */

@interface Bar
{
  int iVar;
}
@property (ivar) int FooBar;
@end

@implementation Bar
@property (readonly) int FooBar; /* { dg-error "property \\'FooBar\\' is \\'readonly\\' in implementation but not in interface" } */

@end
