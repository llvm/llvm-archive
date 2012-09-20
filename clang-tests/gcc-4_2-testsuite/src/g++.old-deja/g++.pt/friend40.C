// { dg-do assemble  }
// Origin: Jason Merrill <jason@cygnus.com>

template<class T> struct A
{
  friend void f ();
};

A<short> a;
A<int> b;

