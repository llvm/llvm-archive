// PR c++/17413

template <typename T> void foo() {}

int main () {
  struct S {};
  foo<S> (); // { dg-warning "" { xfail *-*-* } } currently eaten by SFINAE.
}
