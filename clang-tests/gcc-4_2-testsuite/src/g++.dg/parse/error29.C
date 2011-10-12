// PR c++/25637
// { dg-options "-fno-show-column" }

struct A { 
  void foo();
  A(); 
  void operator delete(void *);
};
struct B { 
  friend void A::foo() {} // { dg-error "friend function definition cannot be qualified" }
  friend void A::operator delete(void*) {} // { dg-error "friend function definition cannot be qualified" }
  friend A::A() {} // { dg-error "friend function definition cannot be qualified" }
};
