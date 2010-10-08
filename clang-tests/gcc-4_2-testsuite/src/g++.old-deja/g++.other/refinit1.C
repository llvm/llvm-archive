// { dg-do assemble  }
// Test that we don't allow multiple user-defined conversions in reference
// initialization.

struct B { };

struct A {   // { dg-error "note" }
  A (const B&);   // { dg-error "note" }
};

struct C {
  operator B ();   // { dg-error "note" }
};

C c;

const A& ref (c);	// { dg-error "no viable conversion from" }
