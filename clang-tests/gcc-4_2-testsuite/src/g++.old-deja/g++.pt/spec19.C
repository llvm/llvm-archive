// { dg-do assemble }
// { dg-options "-Wno-c++11-extensions" }

template<class T> T f(T o) { return o; }
template<> int f(int o)    { return o; }
template int f(int);
