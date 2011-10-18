namespace N {
  template <typename T>
  struct S {
    void f() {} // { dg-warning "explicit instantiation refers here" }
  };
  namespace I {
    template void S<double>::f(); // { dg-error "namespace" }
  }
}

namespace K {
  template void N::S<int>::f(); // { dg-error "namespace" }
}
