/* APPLE LOCAL file radar 4705250 */
/* @defs is allowed with -fobjc-atdefs option. */
/* Compile with no error or warning. */
/* { dg-options "-mmacosx-version-min=10.5 -fobjc-abi-version=2 -fobjc-atdefs" } */
/* { dg-do compile } */
/* APPLE LOCAL radar 4894756 */
/* { dg-skip-if "" { *-*-darwin* } { "-m64" } { "" } } */

#include <stdlib.h>
#include <objc/objc.h>
#include <objc/Object.h>

extern void abort(void);

@interface A : Object
{
  @public
    int a;
}
@end

struct A_defs 
{
  @defs(A);	
};

@implementation A
- init 
{
  a = 42;
  return self;
}
@end


int main() 
{
  A *a = [A initialize];
  struct A_defs *a_defs = (struct A_defs *)a;
  
  if (a->a != a_defs->a)
    abort ();	
  
  return 0;
}
