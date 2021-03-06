/* APPLE LOCAL file radar 4695101 - radar 6064186 */
/* Test that @implementation <protoname> syntax generates metadata for properties 
   declared in @protocol, as well as those declared in the @interface. */
/* { dg-do compile { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* APPLE LOCAL radar 4899595 */
/* { dg-options "-Wno-objc-root-class -mmacosx-version-min=10.6 -m64" } */

@protocol GCObject
@property(readonly) unsigned long int instanceSize;
@property(readonly) long referenceCount;
@property(readonly) const char *description;
@end

@interface GCObject <GCObject> {
    Class       isa;
}
@end

@implementation GCObject 
@dynamic instanceSize;
@dynamic description;
@dynamic referenceCount;
@end

/* { dg-final { scan-assembler "l_OBJC_\\\$_PROP_LIST_GCObject:" } } */
/* { dg-final { scan-assembler "l_OBJC_\\\$_PROP_LIST_GCObject15:" } } */
