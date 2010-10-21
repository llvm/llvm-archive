// PR c++/17413

template <typename T> void foo() {}

int main () {
  struct S {};
  foo<S> (); // { dg-warning "template argument uses local type" }
}
