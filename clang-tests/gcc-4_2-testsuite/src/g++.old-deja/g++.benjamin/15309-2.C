// { dg-do assemble  }
// { dg-options "-Wnon-virtual-dtor -Weffc++" }
// 981203 bkoz
// g++/15309

class bermuda {
public:
  virtual int func1(int); 
  ~bermuda();	// { dg-warning "'bermuda' has virtual functions but non-virtual destructor" }
};
