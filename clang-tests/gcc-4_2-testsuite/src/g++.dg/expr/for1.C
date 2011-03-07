// PR c++/13663

struct S {
  void f();
};

void g(int); // { dg-error "candidate function"}
void g(double); // { dg-error "candidate function"}

void h () {
  S s;
  for (;;s.f); // { dg-error "" }
  for (;;g); // { dg-error "" }
}
