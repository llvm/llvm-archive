// PR c++/29039

struct S {
  int &r; // { dg-error "reference" }
};

S f () {
  return S (); // { dg-error "in value-initialization" }
}


