/* APPLE LOCAL file radar 4805321 */
/* Test for a Synthesized Property to be a 'byref' property by default*/
/* { dg-options "-fobjc-new-property" } */
/* { dg-do compile { target *-*-darwin* } } */

@interface NSPerson
{
  id ivar;
}
@property(retain) id firstName;
@end

@implementation NSPerson
@synthesize firstName=ivar;
@end
/* { dg-final { scan-assembler "object_setProperty_byref" } } */
/* { dg-final { scan-assembler "object_getProperty_byref" } } */
