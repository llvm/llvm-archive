/* APPLE LOCAL file radar 4816280 */
/* Test that gcc warns when property declared in class has no implementation declarative. */
/* { dg-options "-fobjc-new-property -mmacosx-version-min=10.5" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-options "-fobjc-new-property" { target arm*-*-darwin* } } */
/* { dg-do compile { target *-*-darwin* } } */

@interface Subclass 
{
    int ivar;
}
@property int ivar;  /* { dg-warning "property 'ivar' requires method '-ivar' to be defined" } */
                     /* { dg-warning "property 'ivar' requires the method 'setIvar:' to be defined" "" { target *-*-* } 11 } */
@end

@implementation Subclass /* { dg-error "" } */
                         /* { dg-error "" "" { target *-*-* } 15 } */

@end 

int main (void) {
  return 0;
}
