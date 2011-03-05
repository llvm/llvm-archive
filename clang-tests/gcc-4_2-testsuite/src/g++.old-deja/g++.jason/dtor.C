// { dg-do assemble  }
struct A {
  ~A();
};

struct B { // { dg-error "declared here" }
  ~B();
};

int main()
{
  A a;
  a.~B();			// { dg-error "does not match the type" }
}
