/* Tests of duplication.  */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface class1
- (int) meth1;
- (void) meth1;  /* { dg-error "duplicate declaration of method .\\-meth1." } */
@end

__attribute__((objc_root_class)) @interface class2
+ (void) meth1;
+ (int) meth1;  /* { dg-error "duplicate declaration of method .\\+meth1." } */
@end

__attribute__((objc_root_class)) @interface class3
- (int) meth1;
@end

@implementation class3
- (int) meth1 { return 0; } /* { dg-error "previous definition" } */
- (int) meth1 { return 0; } /* { dg-error "redefinition of" } */
@end

__attribute__((objc_root_class)) @interface class4
+ (void) meth1;
@end

@implementation class4
+ (void) meth1 {} /* { dg-error "previous definition" } */
+ (void) meth1 {} /* { dg-error "redefinition of" } */
@end
