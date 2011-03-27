// PR c++/3882
// Verify that variable initialization can be
// self-referencing inside a template function.
/* { dg-options "-Wno-uninitialized" } */

int foo(int);

template <typename T>
void bar(const T&)
{
  int i = foo(i);
}

void quus()
{
  bar(0);
}
