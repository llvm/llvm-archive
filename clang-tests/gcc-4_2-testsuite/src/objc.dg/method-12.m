/* Contributed by Igor Seleznev <selez@mail.ru>.  */
/* This used to be broken.  */

#include <objc/objc.h>

__attribute__((objc_root_class)) @interface A
+ (A *)currentContext;
@end

__attribute__((objc_root_class)) @interface B
+ (B *)currentContext;
@end

int main()
{
    [A currentContext];  /* { dg-bogus "multiple declarations" }  */
    return 0;
}

@implementation A
+ (A *)currentContext { return nil; }
@end
@implementation B
+ (B *)currentContext { return nil; }
@end
