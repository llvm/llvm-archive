/* APPLE LOCAL file radar 4625843 */
/* Test that we generate warning for property type mismatch and object_setProperty_bycopy's
   prototype. */
#include <Foundation/Foundation.h>
#include <stddef.h>

@interface Test : NSObject
@end

@implementation Test
@end

@interface Link : NSObject
@property(bycopy, ivar) Test* test;
@property(ivar) NSString *string;
@end

@implementation Link @end /* { dg-warning "class \'Test\' does not implement the \'NSCopying\' protocol" } */

int main() {
    Test *test = [Test new];
    Link *link = [Link new];
    return 0;
}
