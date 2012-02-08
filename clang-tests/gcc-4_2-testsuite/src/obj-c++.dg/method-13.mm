/* Check if finding multiple signatures for a method is handled gracefully.  Author:  Ziemowit Laski <zlaski@apple.com>  */
/* { dg-options "-Wstrict-selector-match" } */
/* { dg-do compile } */
/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"

@interface Class1
- (void)setWindow:(Object *)wdw; /* { dg-warning "using" } */
@end

@interface Class2
- (void)setWindow:(Class1 *)window; /* { dg-warning "also found" } */
@end

id foo(void) {
  Object *obj = [[Object alloc] init];
  id obj2 = obj;
  [obj setWindow:nil];  /* { dg-warning "Object. may not respond to .setWindow" } */
  [obj2 setWindow:nil]; /* { dg-warning "multiple methods named .setWindow.. found" } */
  return obj;
}
