template <class T >
struct S
{
  S() : S() {} // { dg-error "Delegating constructors are permitted only in C++0x" }
};

S<int> s; // { dg-error "in instantiation" }
