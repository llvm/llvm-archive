/* APPLE LOCAL file 4712269 */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>

@interface SomeClass : Object
@property (ivar) int myValue;
@end

@implementation SomeClass
@property (ivar) int myValue;
@end

int main()
{
    int val;
    SomeClass *o = [SomeClass new];
    o.myValue = -1;
    val = o.myValue++; /* val -1, o.myValue 0 */
    val += o.myValue--; /* val -1. o.myValue -1 */
    val += ++o.myValue; /* val -1, o.myValue 0 */
    val += --o.myValue; /* val -2, o.myValue -1 */
    return ++o.myValue + (val+2);
}

