template <typename T>
void f(int, T (*)() = 0); // { dg-warning "candidate template" }

void g() {
  typedef int A[2];
  f<A>(0); // { dg-error "" }
  typedef void F();
  f<F>(0); // { dg-error "" }
}
