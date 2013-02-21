// { dg-do assemble  }

// Adapted from testcase by Corey Kosak <kosak@cs.cmu.edu>

template<class T>
struct moo_t {
  struct cow_t {};
};

template<class T> void foo(typename moo_t<T>::cow_t) {} // { dg-error "candidate template ignored: couldn't infer" }

template<class T> void foo(moo_t<T>) { // { dg-error "candidate template ignored: could not match" }
  typename moo_t<T>::cow_t p;
  foo(p); // { dg-error "no matching function for call to 'foo'" }
}

int main() {
  moo_t<int> x;
  foo(x); // { dg-error "in instantiation of function template specialization" }
}
