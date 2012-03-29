// Origin: Volker Reichelt <reichelt@igpm.rwth-aachen.de>
// PR c++/18731

template<typename T> struct T::A {}; // { dg-error "nested name specifier 'T::' for declaration does not refer into a class, class template or class template partial specialization" }
