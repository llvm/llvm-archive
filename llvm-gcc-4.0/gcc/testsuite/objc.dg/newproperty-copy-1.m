/* APPLE LOCAL file radar 4805321 */
/* Test that appropriate warning/erros are issued on mis-use of bycopy attibute
   on a property. */
/* { dg-options "-fobjc-new-property" } */
/* { dg-do compile { target *-*-darwin* } } */

@interface INTF
{
	INTF* IVAR;
}
@end

@interface NSPerson
{
  INTF * ivar;
}
@property(copy) INTF * firstName;
@end

@implementation NSPerson
@synthesize firstName=ivar;
@end  	/* { dg-warning "class \'INTF\' does not implement the \'NSCopying\' protocol" } */

@interface INTF (CAT)
@property(copy) INTF* Name; 
@property(copy) INTF* title;
@end

@implementation INTF (CAT)
@synthesize title=IVAR;
@dynamic Name;
@end	/* { dg-warning "class \'INTF\' does not implement the \'NSCopying\' protocol" } */
