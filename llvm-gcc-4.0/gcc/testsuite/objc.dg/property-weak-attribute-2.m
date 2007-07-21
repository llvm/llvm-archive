/* APPLE LOCAL file radar 4621020 */
/* Test a variety of error reporting on mis-use of 'weak' attribute */
/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-fobjc-gc" } */

@interface INTF
{
  id IVAR;
}
@property (weak, ivar=IVAR, bycopy) id pweak;
@end	/* { dg-error "existing ivar \'IVAR\' for a \'weak\' property must be __weak" } */
/* { dg-error "\'weak\' and \'bycopy\' or \'byref\' attributes both cannot" "" { target *-*-* } 11 } */

@implementation INTF
@property (weak, ivar=IVAR, bycopy) id pweak;
@end

