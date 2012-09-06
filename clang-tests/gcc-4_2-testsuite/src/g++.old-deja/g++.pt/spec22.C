// { dg-do assemble  }

template <class T>
struct S
{
  template <class U>
  void f();
};


template <class T> 
template <>
void S<T>::f<int> ()  // { dg-error "cannot specialize" }
{
}
