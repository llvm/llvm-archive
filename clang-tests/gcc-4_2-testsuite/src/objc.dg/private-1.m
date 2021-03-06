/* APPLE LOCAL file radar 4133425 */
/* Test errors for accessing @private and @protected variables.  */
/* Author: Nicola Pero <nicola@brainstorm.co.uk>.  */
/* { dg-do compile } */
#include <objc/objc.h>

__attribute__((objc_root_class)) @interface MySuperClass
{
@private
  int private;

@protected
  int protected;

@public
  int public;
}
- (void) test;
@end

@implementation MySuperClass
- (void) test
{
  private = 12;   /* Ok  */
  protected = 12; /* Ok  */
  public = 12;    /* Ok  */
}
@end


@interface MyClass : MySuperClass 
@end

@implementation MyClass
- (void) test
{
  /* Private variables simply don't exist in the subclass.  */
  private = 12;  /* { dg-error "instance variable" } */
  /* { dg-error "undeclared" "" { target *-*-* } { 38 } } */
  /* { dg-error "function it appears in" "" { target *-*-* } { 38 } } */

  protected = 12; /* Ok  */
  public = 12;    /* Ok  */
}
@end

int main (void)
{
  MyClass *m = nil;
  
  if (m != nil)
    {
      int access;

      access = m->private;   /* { dg-error "is @private" }  */
      access = m->protected; /* { dg-error "is @protected" }  */
      access = m->public;    /* Ok  */
    }

  return 0;
}
