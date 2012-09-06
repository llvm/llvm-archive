// { dg-do assemble  }

template<class T> class D
{
	public:
		int f();
};

template<class T> int D<T>::f()
{
	return sizeof(T);
}

template<template<class> class D,class E> class C // { dg-warning "" } template is declared here
{
		D d;			// { dg-error "" } D is a template
	public:
		int f();
};

template<template<class> class D,class E> int C<D,E>::f()
{
	return d.f();
}

int main()
{
	C<D,int> c;
	c.f();
}
