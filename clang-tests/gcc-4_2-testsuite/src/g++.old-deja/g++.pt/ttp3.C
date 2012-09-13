// { dg-do assemble  }

template<class E,class F> class D  // { dg-error "" }
{
};

template<template<class> class D,class E> class C  // { dg-error "" }
{
};

int main()
{
	C<D,int> c;		// { dg-error "" } param list not match/sees it as not having a type
}
