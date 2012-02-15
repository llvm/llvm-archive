
/* Ensure that we indeed cannot obtain the value of a message send
   if the chosen method signature returns 'void'.  There used to
   exist a cheesy hack that allowed it.  While at it, check that
   the first lexically occurring method signature gets picked
   when sending messages to 'id'.  */ 
/* Contributed by Ziemowit Laski <zlaski@apple.com>  */
/* { dg-do compile } */

#include <objc/objc.h>

@interface Object1
- (void)initWithData:(Object1 *)data; /* { dg-warning "using" } */
@end

@interface Object2
- (id)initWithData:(Object1 *)data; /* { dg-warning "also found" } */
@end

@interface Object3
- (id)initWithData:(Object2 *)data; /* { dg-warning "also found" } */
@end

void foo(void) {
  id obj1, obj2 = 0;
  obj2 = [obj1 initWithData: obj2]; /* { dg-error "assigning to .id. from incompatible type" } */
  /* { dg-warning "multiple methods named .initWithData" "" { target *-*-* } 26 } */
}
