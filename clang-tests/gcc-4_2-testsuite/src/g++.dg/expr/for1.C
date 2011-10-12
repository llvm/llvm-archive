// PR c++/13663

struct S {
  void f();
};

void g(int); // { dg-error "possible target for call" }
void g(double); // { dg-error "possible target for call" }

void h () {
  S s;
  for (;;s.f); // { dg-error "reference to non-static member function must be called" }
  for (;;g); // { dg-error "reference to overloaded function could not be resolved" }
}
