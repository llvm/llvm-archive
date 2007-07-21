/* APPLE LOCAL file radar 4558088 */
/* Test that property declared in interface and implementation have
   identical types. */
/* { dg-do compile { target *-*-darwin* } } */

#include <objc/Object.h>

@interface GCObject {
}

@property(readonly) Class class;
@property(readonly) unsigned int instanceSize;
@property(readonly) long referenceCount;
@property(readonly) BOOL finalized;
@property(readonly) const char *description;

@end

@interface GCPerson : GCObject
@property int* age;
@property(copies)  const char *name;
@end

@implementation GCPerson
@property int age;	/* { dg-error "property \\'age\\' has conflicting type" } */
@property(copies) const char *name;
@end

