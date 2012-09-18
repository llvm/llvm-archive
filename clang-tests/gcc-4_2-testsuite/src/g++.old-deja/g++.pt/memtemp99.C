// { dg-do assemble  }
// Origin: bitti@cs.tut.fi

template<typename T, unsigned int N> // { dg-error "" }
class Vector
{
public:
  template<unsigned int I> // { dg-error "" }
  class Vector<T,N>::CommaInit { }; // { dg-error "" } invalid definition
};
