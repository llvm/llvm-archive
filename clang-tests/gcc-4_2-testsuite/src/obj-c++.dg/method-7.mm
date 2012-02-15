/* Check if sending messages to "underspecified" objects is handled gracefully.  */
/* Author: Ziemowit Laski <zlaski@apple.com>.  */

/* { dg-do compile } */

@class UnderSpecified; /* { dg-warning "forward declaration of class here" } */
typedef struct NotAClass {
  int a, b;
} NotAClass;

void foo(UnderSpecified *u, NotAClass *n) {
  [n nonexistent_method];   /* { dg-warning "receiver type .NotAClass .. is not .id. or interface pointer, consider casting it to .id" } */
                            /* { dg-warning "instance method ..nonexistent_method. not found" "" { target *-*-* } 12 } */
  [NotAClass nonexistent_method]; /* { dg-error "receiver type .NotAClass. is not an Objective.C class" } */
  [u nonexistent_method]; /* { dg-warning "instance method ..nonexistent_method. not found" } */
  [UnderSpecified nonexistent_method]; /* { dg-warning "receiver .UnderSpecified. is a forward class and corresponding .interface may not exist" } */
                            /* { dg-warning "class method ..nonexistent_method. not found" "" { target *-*-* } 16 } */
}
