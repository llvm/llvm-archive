/* APPLE LOCAL file 4805321 */
/* { dg-do run { target *-*-darwin* } } */
/* { dg-options "-fobjc-new-property -lobjc" } */

#include <objc/objc.h>
#include <objc/Object.h>


@protocol GCObject
@property (assign) int CCCC;
@end

@protocol DerivedGCObject <GCObject>
@property (assign) int Dclass;
@end

@interface GCObject  : Object <DerivedGCObject> {
    int ifield;
    int iDclass, iOwnClass;
}
@property (assign) int OwnClass;
@end

@implementation GCObject : Object
@synthesize CCCC=ifield, Dclass=iDclass, OwnClass=iOwnClass;
@end

int main(int argc, char **argv) {
    GCObject *f = [GCObject new];
    f.CCCC = 5;
    f.Dclass = 1;
    f.OwnClass = 3;
    return f.CCCC + f.Dclass  + f.OwnClass - 9;
}
