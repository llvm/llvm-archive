// { dg-do assemble  }

namespace A {
  int i = 1;	// { dg-warning "" } 
}

namespace B {
  int j = i;	// { dg-error "" } 
}
