/* Test errors for assignments and comparisons between ObjC and C++ types.  */
/* Author: Nicola Pero <nicola@brainstorm.co.uk>.  */
/* { dg-options "-Wno-empty-body" } */
/* { dg-do compile } */

#include <objc/objc.h>

/* The NeXT runtime headers do not define NULL.  */
#ifndef NULL
#define NULL ((void *)0)
#endif

@protocol MyProtocol
- (void) method;
@end

@interface MyClass
@end

int main()
{
  id obj = nil;
  id <MyProtocol> obj_p = nil;
  MyClass *obj_c = nil;
  Class obj_C = Nil;
  
  int i = 0;
  int *j = (int *)NULL;

  /* These should all generate warnings.  */
  
  obj = i; /* { dg-error "assigning to .{4} from incompatible type" } */
  obj = j; /* { dg-error "assigning to .{4} from incompatible type" } */

  obj_p = i; /* { dg-error "assigning to .{16} from incompatible type" } */
  obj_p = j; /* { dg-error "assigning to .{16} from incompatible type" } */
  
  obj_c = i; /* { dg-error "assigning to .{11} from incompatible type" } */
  obj_c = j; /* { dg-error "assigning to .{11} from incompatible type" } */

  obj_C = i; /* { dg-error "assigning to .{7} from incompatible type" } */
  obj_C = j; /* { dg-error "assigning to .{7} from incompatible type" } */
  
  i = obj;   /* { dg-error "assigning to .{5} from incompatible type" } */
  i = obj_p; /* { dg-error "assigning to .{5} from incompatible type" } */
  i = obj_c; /* { dg-error "assigning to .{5} from incompatible type" } */
  i = obj_C; /* { dg-error "assigning to .{5} from incompatible type" } */
  
  j = obj;   /* { dg-error "assigning to .{7} from incompatible type" } */
  j = obj_p; /* { dg-error "assigning to .{7} from incompatible type" } */
  j = obj_c; /* { dg-error "assigning to .{7} from incompatible type" } */
  j = obj_C; /* { dg-error "assigning to .{7} from incompatible type" } */
  
  if (obj == i) ; /* { dg-error "comparison between pointer and integer" } */
  if (i == obj) ; /* { dg-error "comparison between pointer and integer" } */
  if (obj == j) ; /* { dg-warning "comparison of distinct pointer types" } */
  if (j == obj) ; /* { dg-warning "comparison of distinct pointer types" } */

  if (obj_c == i) ; /*{ dg-error "comparison between pointer and integer" }*/
  if (i == obj_c) ; /*{ dg-error "comparison between pointer and integer" }*/
  if (obj_c == j) ; /* { dg-warning "comparison of distinct pointer types" } */
  if (j == obj_c) ; /* { dg-warning "comparison of distinct pointer types" } */

  if (obj_p == i) ; /*{ dg-error "comparison between pointer and integer" }*/
  if (i == obj_p) ; /*{ dg-error "comparison between pointer and integer" }*/
  if (obj_p == j) ; /* { dg-warning "comparison of distinct pointer types" } */
  if (j == obj_p) ; /* { dg-warning "comparison of distinct pointer types" } */

  if (obj_C == i) ; /*{ dg-error "comparison between pointer and integer" }*/
  if (i == obj_C) ; /*{ dg-error "comparison between pointer and integer" }*/
  if (obj_C == j) ; /* { dg-warning "comparison of distinct pointer types" } */
  if (j == obj_C) ; /* { dg-warning "comparison of distinct pointer types" } */

  return 0;
}
