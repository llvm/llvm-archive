// { dg-do compile  }
// GROUPS passed templates
template <class T>
T foo(T* t);

template <>
int foo<char>(char c); // { dg-error "" } does not match declaration.

template <>      // { dg-error "" }
int bar<char>(); // { dg-error "" } no template bar.
		 // { dg-error "" "" { target *-*-* } 10 }  bogus code
