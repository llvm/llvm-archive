/* APPLE LOCAL file radar 4625843 */
/* Test that appropriate warning/erros are issued on mis-use of bycopy attibute
   on a property. */
/* { dg-do compile { target *-*-darwin* } } */

@interface INTF
{
	INTF* IVAR;
}
@end

@interface NSPerson
@property(ivar, bycopy) INTF * firstName;
@end

@implementation NSPerson
@property(ivar, bycopy) INTF * firstName;
@end  
/* { dg-warning "class \'INTF\' does not implement the \'NSCopying\' protocol" "" { target *-*-* } 18 } */

@interface INTF (CAT)
@property(ivar, bycopy) INTF* Name; /* { dg-error "in category only ivar=name is valid" } */
@property(ivar=IVAR, bycopy) INTF* title;
@end

@implementation INTF (CAT)
@property(ivar, bycopy) INTF* title; /* { dg-error "property \'title\'\'s interface and implementation have conflicting" } */
@end
