/* APPLE LOCAL file 4531482 */
/* Test for hybrid abi producing module and symbol meta data. */
/* { dg-options "-fobjc-abi-version=3" } */
/* { dg-do compile } */

/* One-line substitute for objc/objc.h */
typedef struct objc_object { struct objc_class *class_pointer; } *id;

@protocol NSObj 
- (void)someMethod;
@end

@protocol NSCopying 
- (void)someOtherMethod;
@end

@interface NSObject <NSObj>
- (void)someMethod; 
@end

@implementation NSObject
- (void)someMethod {}
@end

@protocol Booing <NSObj>
- (void)boo;
@end

@interface Boo: NSObject <Booing>  // protocol has only one parent
@end

@implementation Boo
- (void)boo {}
@end

@protocol Fooing <NSCopying, NSObj>  // Fooing has two parent protocols
- (void)foo;
@end

@interface Foo: NSObject <Fooing>
@end

@implementation Foo
- (void)foo {}
- (void)someOtherMethod {}
@end

int foo(void) {
  id<Booing, Fooing> stupidVar;
  [stupidVar boo];
  [stupidVar foo];
  [stupidVar anotherMsg]; /* { dg-warning ".\\-anotherMsg. not found in protocol" } */
       /* { dg-warning "no .\\-anotherMsg. method found" "" { target *-*-* } 52 } */
  return 0;
}

/* { dg-warning "Messages without a matching method signature" "" { target *-*-* } 0 } */
/* { dg-warning "will be assumed to return .id. and accept" "" { target *-*-* } 0 } */
/* { dg-warning ".\.\.\.. as arguments" "" { target *-*-* } 0 } */
/* { dg-final { scan-assembler "L_OBJC_MODULES" } } */ 
/* { dg-final { scan-assembler "L_OBJC_SYMBOLS" } } */ 
