/* APPLE LOCAL file 4512786 */
/* { dg-do run } */
#include <objc/objc.h>
#include <objc/Object.h>

@interface M : Object
- (void) foo;
@end

@implementation M : Object
- (void) foo
{
    @try {
    } @catch (Object *localException) {
    }

}
@end

int main()
{
	M *p = [M new];
	[p foo];
	return 0;
}
