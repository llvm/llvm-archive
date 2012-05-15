// PR c++/5657
// Origin: Volker Reichelt <reichelt@igpm.rwth-aachen.de>
// { dg-do compile }

template<typename T> struct A { A(B); }; // { dg-warning "template is declared here" }
template<typename T> A<T>::A(B) {} // { dg-error "" }
