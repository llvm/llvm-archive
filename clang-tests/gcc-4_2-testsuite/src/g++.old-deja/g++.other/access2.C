// { dg-do assemble  }
// Based on a testcase in the Standard, submitted by several people

class Outer {
  typedef int T;
  struct Inner {
    T i; 
    void f() {
      T j; 
    }
  };
};
