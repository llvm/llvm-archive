struct B {
  void g() { }
};

struct A {
  void f() {
    B &b = b; // { dg-warning "reference 'b' is not yet bound to a value when used within its own initialization" }
    b.g();
  }
};

int main(void) { }
