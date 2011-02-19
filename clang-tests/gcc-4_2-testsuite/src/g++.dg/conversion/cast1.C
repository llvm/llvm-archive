// PR c++/10841

int main() {
  class Base { 
  public: 
    int i, j, k; 
    void f(); }; /* { dg-error "has internal linkage but is not defined" } */

  class Derived : private Base { 
  public: 
    int m, n, p; 
    void g();  /* { dg-error "has internal linkage but is not defined" } */
  };

  Derived derived;
  Base &base = (Base &)derived;
  (int Base::*)&Derived::n;
  (int Derived::*)&Base::j;
  (void (Base::*)(void))&Derived::g; /* { dg-error "note: used here" } */
  (void (Derived::*)(void))&Base::f; /* { dg-error "note: used here" } */
}

