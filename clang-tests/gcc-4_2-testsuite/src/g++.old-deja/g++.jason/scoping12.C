// { dg-do assemble  }
void f ()
{
  struct A {
    friend void g (); // { dg-error "no matching function found in local scope"}
  };
}
void h () {
  g ();				// { dg-error "use of undeclared identifier"}
}
