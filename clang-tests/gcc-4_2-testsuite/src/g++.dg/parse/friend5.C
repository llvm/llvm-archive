// PR c++/23694
 
extern "C" struct A /* { dg-warning "ignored on this declaration" } */
{
  friend void foo(int) {} // { dg-error "declaration" }
  friend void foo() {} // { dg-error "foo" }
};
