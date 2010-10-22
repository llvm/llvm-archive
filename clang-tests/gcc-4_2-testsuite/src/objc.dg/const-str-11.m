/* Test if ObjC constant string layout is checked properly, regardless of how
   constant string classes get derived.  */
/* Contributed by Ziemowit Laski <zlaski@apple.com>  */

/* APPLE LOCAL file 4149909 */
/* { dg-options "-m32 -fnext-runtime -fno-constant-cfstrings -fconstant-string-class=XStr" } */

#include <objc/Object.h>

@interface XString: Object {
@protected
    char *bytes;
}
@end

@interface XStr : XString {
@public
    unsigned int len;
}
@end

/* APPLE LOCAL begin objc2 */
#if OBJC_API_VERSION >= 2
extern Class _XStrClassReference;
#else
extern struct objc_class _XStrClassReference;
#endif
/* APPLE LOCAL end objc2 */

const XStr *appKey = @"MyApp";

/* { dg-final { scan-assembler ".section\t__OBJC,__cstring_object" } } */
/* { dg-final { scan-assembler ".long\t__XStrClassReference\n\t.long\t.*\n\t.long\t5" } } */
