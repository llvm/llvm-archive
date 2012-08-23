/* APPLE LOCAL file radar 5232840 - radar 5398274 */
/* Test that no warning is issued on 'unused' "_value" parameter even though it is used. */
/* { dg-options "-Wno-objc-root-class -Wunused-parameter -mmacosx-version-min=10.5" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-options "-Wno-objc-root-class -Wunused-parameter" { target arm*-*-darwin* } } */
/* { dg-do compile { target *-*-darwin* } } */
@interface MyClass 
{
@private
        id retainValue;
        id copiedValue;
        id assignValue;
        id readOnlyValue;
        int foo;
}
@property(readwrite, retain) id retainValue;
@property(readwrite, assign) id assignValue;
@property(readwrite, copy) id copiedValue;
@property(readonly) id readOnlyValue;

@property(readwrite) int foo;
@end


@implementation MyClass
@synthesize retainValue;
@synthesize assignValue;
@synthesize copiedValue;
@synthesize readOnlyValue;

@synthesize foo;
@end
