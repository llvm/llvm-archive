/* Test for graceful handling of anonymous ivars.  */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface Foo {
   unsigned char : 1;
   int e: 3;
   signed: 2;
   float f;
}
@end

@implementation Foo
@end

