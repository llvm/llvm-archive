/* Test for assigning compile-time constant-string objects to static variables.  */
/* Contributed by Ziemowit Laski <zlaski@apple.com>  */

/* { dg-options "-fnext-runtime -fconstant-string-class=Foo -lobjc" } */
/* { dg-do run { target *-*-darwin* } } */


#include <stdlib.h>
/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"

@interface Foo: Object {
  char *cString;
  unsigned int len;
}
@end

/* APPLE LOCAL begin objc2 */
#if OBJC_API_VERSION >= 2
Class _FooClassReference;
#else
struct objc_class _FooClassReference;
#endif
/* APPLE LOCAL end objc2 */

@implementation Foo : Object
- (char *)customString {
  return cString;
}
@end

static const Foo *appKey = @"MyApp";
static int CFPreferencesSynchronize (const Foo *ref) {
  return ref == appKey;
}

static void PrefsSynchronize(void)
{
  if(!CFPreferencesSynchronize(appKey))
    abort();
}

int main () {
  PrefsSynchronize();
  return 0;
}
