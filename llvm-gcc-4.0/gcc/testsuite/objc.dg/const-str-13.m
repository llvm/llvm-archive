/* APPLE LOCAL file 4080358 */
/* Test if ObjC strings play nice with -fwritable-strings.  */
/* Author: Ziemowit Laski  */

/* { dg-options "-fno-constant-cfstrings -fwritable-strings -fconstant-string-class=Foo" } */
/* { dg-do run { target *-*-darwin* } } */
/* { dg-skip-if "" { *-*-darwin* } { "-m64" } { "" } } */

/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"
#include <stdlib.h>
#include <memory.h>

@interface Foo: Object {
  char *cString;
  unsigned int len;
}
- (char *)c_string;
@end

/* Unavailable in objc 2.0, deprecated in Leopard */
struct objc_class _FooClassReference; /* { dg-warning "warning: \'objc_class\' is deprecated" } */

static Foo *foobar = @"Apple";

@implementation Foo
- (char *)c_string {
  return cString;
}
@end

int main(void) {
  char *c, *d;

  /* Initialize the metaclass.  */
  memcpy(&_FooClassReference, objc_getClass("Foo"), sizeof(_FooClassReference));
  c = [foobar c_string];
  d = [@"Hello" c_string];

  if (*c != 'A')
    abort ();

  if (*d != 'H')
    abort ();

  return 0;
}
