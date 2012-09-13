// { dg-do assemble  }

template <int i> class C {}; // { dg-error "" }
template <template <long> class TT> class D {}; // { dg-error "" }

int main()
{
	D<C> d;		// { dg-error "" } args not match
}
