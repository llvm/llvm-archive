// { dg-do compile }
// Origin: Giovanni Bajo <giovannibajo at gcc dot gnu dot org>
// DR213: Lookup in dependent base classes

// We should emit an error at *instantiation* time because g(t) can't be
//  resolved to any function.

template <class T> struct A : T {
  void h(T t) {
    f(t);
    g(t);     // { dg-error "use of undeclared identifier" }
  }
};

struct B {
  void f(B);
  void g(B) {} // { dg-error "must qualify identifier to find this declaration in dependent base class" }
};

void f(B) {}

int main()
{
  A<B> ab;
  B b;
  ab.h(b); // { dg-error "in instantiation of member function" }
}
