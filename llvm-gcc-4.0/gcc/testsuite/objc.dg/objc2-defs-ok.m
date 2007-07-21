/* APPLE LOCAL file radar 4705250 */
/* @defs is allowed with -fobjc-atdefs option. */
/* Compile with no error or warning. */
/* { dg-options "-fobjc-abi-version=2 -fobjc-atdefs" } */
/* { dg-do compile } */
/* { dg-skip-if "" { *-*-darwin* } { "-m64" } { "" } } */

#include <objc/objc.h>
#include <objc/Object.h>

extern void abort(void);

#define CHECK_IF(expr) if(!(expr)) abort();

@interface Base: Object {
@public
  int a;
  float b;
  char c;
}
@end

@interface Derived: Base {
@public
  double d;
  unsigned e;
  id f;
} 
@end

struct Derived_defs {
  @defs(Derived);	
};

@implementation Base
@end
@implementation Derived
@end

int main(void) {
  Derived *derived = [Derived  initialize];
  struct Derived_defs *derived_defs = (struct Derived_defs *)derived;

  CHECK_IF(derived->a == derived_defs->a && derived_defs->a == 123);
  CHECK_IF(derived->b == derived_defs->b && derived_defs->b == (float)1.23);  
  CHECK_IF(derived->c == derived_defs->c && derived_defs->c == 'c');  
  CHECK_IF(derived->d == derived_defs->d && derived_defs->d == (double)123.456);  
  CHECK_IF(derived->e == derived_defs->e && derived_defs->e == 456);  
  CHECK_IF(derived->f == derived_defs->f && derived_defs->f == derived_defs->isa);

  CHECK_IF(((struct { @defs(Derived); } *)derived)->a == 123); 
  return 0;
}
