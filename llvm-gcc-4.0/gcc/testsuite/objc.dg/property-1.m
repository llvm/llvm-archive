/* APPLE LOCAL file radar 4436866 */
/* This program tests use of property provided setter/getter functions. */
/* { dg-options "-std=c99 -lobjc" } */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>

@interface Bar : Object
{
  int iVar;
}
@property (ivar = iVar) int FooBar;
@end

@implementation Bar
@property (ivar = iVar, setter = MySetter:) int FooBar;

- (void) MySetter : (int) value { iVar = value; }

@end

int main(int argc, char *argv[]) {
    Bar *f = [Bar new];
    f.FooBar = 1;

    f.FooBar += 3;

    f.FooBar -= 4;
    return f.FooBar;
}

