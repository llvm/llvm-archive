/* APPLE LOCAL file radar 4805321 */
/* Test that we generate warning for property type mismatch and object_setProperty_bycopy's
   prototype. */
/* { dg-options "-fobjc-gc -fobjc-new-property" } */
/* { dg-do compile { target *-*-darwin* } } */

#include <Foundation/Foundation.h>
#include <stddef.h>

@interface Test : NSObject
@end

@implementation Test
@end

@interface Link : NSObject
{
  Test *itest;
  NSString *istring;
}
@property(copy) Test* test;
@property (assign) NSString *string;
@end

@implementation Link 
@synthesize test=itest, string=istring;

@end  /* { dg-warning "class \\'Test\\' does not implement the \\'NSCopying\\' protocol" } */

int main() {
    Test *test = [Test new];
    Link *link = [Link new];
    return 0;
}
