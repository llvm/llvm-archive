// { dg-do assemble  }
// try throwing overloaded function

void f(int) // { dg-error "" }
{
}

void f(long) // { dg-error "" }
{
}

void g()
{
	throw &f; // { dg-error "" } insufficient contextual information
}
