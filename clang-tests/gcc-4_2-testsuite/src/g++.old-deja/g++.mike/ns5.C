// { dg-do assemble  }
namespace A {
  int i = 1; // { dg-warning "" }
}

int j = i;		// { dg-error "" } 
