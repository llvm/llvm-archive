// PR c++/20552
// Origin: Ivan Godard <igodard@pacbell.net>

template<int> struct A
{
  void foo()
  {
    typedef int T;              
    typedef __typeof__(*this) T;
  }
};

template<typename T>
struct X {
  void foo() {
    typedef T* t1;
    typedef const T* t1;
  }
};

template void X<const int>::foo(); 
