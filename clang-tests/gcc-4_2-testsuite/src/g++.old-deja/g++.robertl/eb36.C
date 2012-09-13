// { dg-do assemble  }
#include <vector>
using namespace std;

template <typename T=float> class foo {
public:
  foo();
  foo(vector<int> v);
private:
  vector<int> v;
  T t;
};

template <typename T>
foo<T>::foo()               :v(),   t() {}
template <typename T=float> // { dg-error "" } default arg for member template
foo<T>::foo(vector<int> v_) :v(v_), t() {}

foo<float> a;
