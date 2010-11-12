template <typename T, bool=T::X> struct A  // { dg-error 2 "" }
{
  int i;
};

template <typename T> struct B : A<T> // { dg-error "" }
{ 
  using A<T>::i; 
};

B<void> b; // { dg-error 2 "" }
