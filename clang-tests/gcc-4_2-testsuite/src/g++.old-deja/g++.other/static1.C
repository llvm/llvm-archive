// { dg-do assemble  }
extern "C" void abort();

struct S
{
  static const int i = 3;  // { dg-error "note" }
};

const int S::i = 2; // { dg-error "redefinition" }
