/* APPLE LOCAL file radar 4712415 */
/* This program tests use of deprecated attribute on property. */
/* { dg-options "-fobjc-new-property" } */
/* { dg-do compile { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>

@interface Bar : Object
{
  int iVar;
}
@property (assign, setter = MySetter:) int FooBar __attribute__ ((deprecated));
- (void) MySetter : (int) value;
@end

@implementation Bar
@synthesize FooBar = iVar;
- (void) MySetter : (int) value { iVar = value; }

@end

int main(int argc, char *argv[]) {
    Bar *f = [Bar new];
    f.FooBar = 1;	/* { dg-warning "\'FooBar\' is deprecated" } */
			/* { dg-warning "\'MySetter:\' is deprecated" "" { target *-*-* } 25 } */
    return f.FooBar;	/* { dg-warning "\'FooBar\' is deprecated" } */
}
