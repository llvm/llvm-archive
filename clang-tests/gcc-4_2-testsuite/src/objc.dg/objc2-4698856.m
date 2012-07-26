/* APPLE LOCAL file radar 4698856 */
/* Test that _objc_empty_cache and _objc_empty_vtable may be user defined. */
/* { dg-options "-mmacosx-version-min=10.5 -fobjc-abi-version=2" { target powerpc*-*-darwin* i?86*-*-darwin* } } */

#include <objc/object.h>

static void * _objc_empty_cache;
static IMP _objc_empty_vtable;

__attribute__((objc_root_class)) @interface Foo
@end

@implementation Foo @end
