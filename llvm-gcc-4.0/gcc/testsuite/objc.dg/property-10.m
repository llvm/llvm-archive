/* APPLE LOCAL file 4564386 */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>


@protocol GCObject
@property (ivar=ifield) int class;
@end

@protocol DerivedGCObject <GCObject>
@property (ivar) int Dclass;
@end

@interface GCObject  : Object <DerivedGCObject> {
    int ifield;
}
@property (ivar) int OwnClass;
@end

@implementation GCObject : Object
@property(ivar=ifield) int class;
@property int Dclass;
@property int OwnClass;
@end

int main(int argc, char **argv) {
    GCObject *f = [GCObject new];
    f.class = 5;
    f.Dclass = 1;
    f.OwnClass = 3;
    return f.class + f.Dclass  + f.OwnClass - 9;
}
