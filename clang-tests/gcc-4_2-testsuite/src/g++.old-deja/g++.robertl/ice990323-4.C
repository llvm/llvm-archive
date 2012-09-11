// { dg-do assemble  }
// try throwing template function name

template <class T> void f(T);

template <class T> void f(T) // { dg-warning "" }
{
}

void g()
{
	throw &f; // { dg-error "" } insufficient contextual information
}
