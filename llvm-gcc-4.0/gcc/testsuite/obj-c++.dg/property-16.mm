/* APPLE LOCAL file radar 4660579 */
/* Test that property can be declared 'readonly' in interface but it can be
   overridden in the implementation and can be assigned to.
*/
/* { dg-options "-lobjc" } */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>


@interface ReadOnly : Object
@property(readonly, ivar) int object;
@property(readonly, ivar) int Anotherobject;
@end

@implementation ReadOnly
@property(ivar) int object;
@property(ivar, setter = myAnotherobjectSetter:) int Anotherobject;
- (void) myAnotherobjectSetter : (int)val {
    _Anotherobject = val;
}
@end

int main(int argc, char **argv) {
    ReadOnly *test = [ReadOnly new];
    test.object = 12345;
    test.Anotherobject = 200;
    return test.object - 12345 + test.Anotherobject - 200;
}

