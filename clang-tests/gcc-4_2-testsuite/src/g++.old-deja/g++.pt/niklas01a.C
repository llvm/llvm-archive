// { dg-do assemble  }

struct A {
  friend struct B : A {		// { dg-error "" } cannot define a type in a friend declaration
    int x;
  };
  int y;
};
