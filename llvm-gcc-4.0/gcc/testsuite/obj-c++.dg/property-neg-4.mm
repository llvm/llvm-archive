/* APPLE LOCAL file radar 4436866 */
/* Property cannot be accessed in class method. */
/* { dg-do compile { target *-*-darwin* } } */

@interface Person 
{
}
@property (ivar) char *fullName;
+ (void) testClass;
@end	

@implementation  Person
@property char *fullName;
+ (void) testClass {
	self.fullName = "MyName"; /* { dg-error "request for member \\'fullName\\' in \\'self\\'" } */
}
@end

