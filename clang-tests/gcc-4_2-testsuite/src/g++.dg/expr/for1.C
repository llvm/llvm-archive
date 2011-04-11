// PR c++/13663

struct S {
  void f();
};

void g(int); // { dg-error "candidate" }
void g(double); // { dg-error "candidate" }

void h () {
  S s;
  for (;;s.f); // { dg-error "a bound member function" }
  for (;;g); // { dg-error "cannot resolve overloaded function" }
}
