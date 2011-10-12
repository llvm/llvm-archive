// { dg-do compile }

// Copyright (C) 2001, 2002 Free Software Foundation, Inc.
// Contributed by Nathan Sidwell 28 Dec 2001 <nathan@codesourcery.com>

// PR 5123. ICE

struct C {
  template<class T> void f(T);
  void g ();
  void g (int);
};

void Foo () {
  C c;

  (c.g) ();
  (c.f) (1);
  
  (c.f<int>) (2);

  c.g;	// { dg-error "reference to non-static member function must be called" }
  c.f;	// { dg-error "reference to non-static member function must be called" }	        
  c.f<int>; // { dg-error "reference to non-static member function must be called" }
  
  c.g == 1; // { dg-error "reference to non-static member function must be called" }
  c.f == 1; // { dg-error "reference to non-static member function must be called" }
  c.f<int> == 1;  // { dg-error "reference to non-static member function must be called" }
}
