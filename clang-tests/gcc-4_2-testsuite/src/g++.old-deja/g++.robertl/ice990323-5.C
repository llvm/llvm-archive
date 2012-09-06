// { dg-do assemble  }
// check attempting to throw an overloaded function

struct A {
	void f(int); // { dg-warning "" } possible target for call
	void f(long); // { dg-warning "" } possible target for call
};

void g()
{
	throw &A::f; // { dg-error "" } insufficient context
}
