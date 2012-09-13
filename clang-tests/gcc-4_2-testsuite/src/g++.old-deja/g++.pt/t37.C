// { dg-do assemble  }

class A {
public:
  A(int);
  A(float);
  ~A();
};

A::A() {		// { dg-error "" } 
}
  
A::A(int) {
}
  
A::~A() {
}
