/* APPLE LOCAL file 4564386 */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>


@protocol GCObject
@property (ivar=ifield) int age;
@end

@protocol DerivedGCObject <GCObject>
@property (ivar) int Dage;
@end

@interface GCObject  : Object <DerivedGCObject> {
    int ifield;
}
@property (ivar) int OwnAge;
@end

@implementation GCObject : Object
@property(ivar=ifield) int age;
@property int Dage;
@property int OwnAge;
@end

int main(int argc, char **argv) {
    GCObject *f = [GCObject new];
    f.age = 5;
    f.Dage = 1;
    f.OwnAge = 3;
    return f.age + f.Dage  + f.OwnAge - 9;
}
