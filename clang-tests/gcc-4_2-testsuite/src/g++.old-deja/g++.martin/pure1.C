// { dg-do assemble  }
class A
{
  public:
    virtual void f(void) = 0; // { dg-error "note" }
     A() {f();}               // { dg-warning "call to pure virtual member" } 
    ~A() {f();}               // { dg-warning "call to pure virtual member" } 
};
