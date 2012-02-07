/* APPLE LOCAL file mainline */
/* { dg-do compile } */


/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"

@protocol Foo
- (id)meth1;
- (id)meth2:(int)arg;
@end

@interface Derived1: Object
@end

@interface Derived2: Object
+ (Derived1 *)new; /* { dg-warning "method is expected to return an instance of its class type" } */
@end

id<Foo> func(void) {
  Object *o = [Object new];
  return o;  /* { dg-error "cannot initialize return object of type .{9} with an lvalue of type" } */
}

@implementation Derived2
+ (Derived1 *)new {
  Derived2 *o = [super new];
  /* { dg-output "overridden method is part of the 'new' method family" } */
  return o;  /* { dg-error "cannot initialize return object of type .+ with an lvalue of type" } */
}
@end

