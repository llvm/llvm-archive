// { dg-do assemble }

// Based on bug report by Simon A. Crase <s.crase@ieee.org>


struct foo {
  template <class T> void bar(); // { dg-error "explicit instantiation refers here" }
};

template void foo::bar<void>(); // { dg-error "explicit instantiation of undefined function template 'bar'" }
