
/* Make sure that bitfield types are printed correctly, and that ivar redeclaration
  (@interface vs. @implementation) checks take the bitfield width into account.  */
/* Author: Ziemowit Laski   <zlaski@apple.com>  */
/* { dg-do compile } */

@interface Base {
  int i;
}
@end

@interface WithBitfields: Base {
  void *isa;     /* { dg-warning "previous definition is here" } */
  unsigned a: 3; /* { dg-warning "previous definition is here" } */
  signed b: 4;   /* { dg-warning "previous definition is here" } */
  int c: 5;      /* { dg-warning "previous definition is here" } */
}
@end

@implementation WithBitfields {
  char *isa;     /* { dg-error "instance variable is already declared" } */
  unsigned a: 5; /* { dg-error "instance variable is already declared" } */
  signed b: 4;   /* { dg-error "instance variable is already declared" } */
  int c: 3;      /* { dg-error "instance variable is already declared" } */
}
@end
