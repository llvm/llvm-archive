/* APPLE LOCAL file radar 4576123 */
/* Test that property's type in its meta-data has the class name. */
/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-fobjc-abi-version=3" } */

@class String;

@interface Base {
  Class isa;
}
@property(readonly, ivar=isa) Class class;
@property (ivar) String *description;

@end
@implementation Base

@property(readonly, ivar=isa) Class class;
@property String *description;

@end
/* { dg-final { scan-assembler "OBJC_PROP_NAME_ATTR_1:\n\t.ascii \".*\\\"String.*" } } */
