// { dg-do assemble  }

template <class T = int> // { dg-error "" } original definition
struct S
{
  template <class U = int>  // { dg-error "" } redefinition of default arg
  friend class S;
};

template struct S<int>;
