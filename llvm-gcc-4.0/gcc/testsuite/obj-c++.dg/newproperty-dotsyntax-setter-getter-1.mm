/* APPLE LOCAL file radar 4805321 */
/* Program to test new dot-syntax to call setter/getter. */
/* { dg-options "-fobjc-new-property -lobjc" } */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>

@interface Bar : Object
{
  int iVar;
}
- (int) FooBar;
- (void) setFooBar : (int) value;
@end

@implementation Bar
- (void) setFooBar : (int) value { iVar = value; }
- (int) FooBar {return iVar; }

@end

int main(int argc, char *argv[]) {
    Bar *f = [Bar new];
    f.FooBar = 1;

    f.FooBar += 3;

    f.FooBar -= 4;
    return f.FooBar;
}

