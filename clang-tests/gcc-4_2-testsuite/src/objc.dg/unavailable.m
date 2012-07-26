/* APPLE LOCAL file "unavailable" attribute 5556192 */

/* { dg-do compile } */

__attribute__((objc_root_class)) @interface Foo
-(void) method3 __attribute__((unavailable));
@end

extern void func3() __attribute__((unavailable));

void test(Foo* obj)
{
  [obj method3];	/* { dg-error "is unavailable" } */

  func3();		/* { dg-error "is unavailable" } */
}
