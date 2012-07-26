/* APPLE LOCAL file 4568791 */
/* Test that ity is OK (no warning) when Derived class does not implement
   the method in protocol as long as its super class does. */
/* { dg-do compile } */

@protocol P
- (void)m;
@end

__attribute__((objc_root_class)) @interface Base
- (void)m;
@end

@interface Derived : Base <P>
@end

@implementation Base
- (void)m {
	return;
}
@end

@implementation Derived
@end
