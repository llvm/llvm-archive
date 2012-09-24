// { dg-do assemble  }

template <class T> class A;
// template <>
class A<int>; // { dg-error "" } missing template header - 
