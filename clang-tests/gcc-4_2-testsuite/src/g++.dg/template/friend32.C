// { dg-do compile }

// Origin: Steven Bosscher <steven@gcc.gnu.org>
//	   Serge Belyshev <belyshev@lubercy.com>

// PR c++/18825: ICE member as friend

template<class T> class A
{
  void f ();			// { dg-error "private" }
};

template<class T> class B
{
  friend void A<T>::f ();	// { dg-error "friend function 'f' is a private member of 'A<int>'" }
};

int f ()
{
  B<int> b;			// { dg-error "instantiation" }
}
