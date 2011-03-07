// { dg-do assemble  }
// Test for proper diagnostics on trying to take the address of a non-static
// member function.

struct A {
  void f (); // { dg-error "candidate function" }
  void f (int); // { dg-error "candidate function" }
  void g ();
};

int main ()
{
  A a;
  &a.f;				// { dg-error "" } overloaded
  &a.g;				// { dg-error "" } can't write a pmf like this
}
