// { dg-do compile }

// Copyright (C) 2001, 2002 Free Software Foundation, Inc.
// Contributed by Nathan Sidwell 28 Dec 2001 <nathan@codesourcery.com>

// PR 5123. ICE

struct C {
  template<class T> void f(T); // { dg-error "candidate function" "" }
  void g (); // { dg-error "candidate function" "" }
  void g (int); // { dg-error "candidate function" "" }
};

void Foo () {
  C c;

  (c.g) ();
  (c.f) (1);
  
  (c.f<int>) (2);

  c.g;			// { dg-error "cannot resolve overloaded function" "" }
  c.f;		        // { dg-error "cannot resolve overloaded function" "" }
  c.f<int>;		// { dg-error "a bound member function may only be called" }
  
  c.g == 1;		// { dg-error "cannot resolve overloaded function" "" }
  c.f == 1;		// { dg-error "cannot resolve overloaded function" "" }
  c.f<int> == 1;	// { dg-error "comparison between pointer and integer" }
}
