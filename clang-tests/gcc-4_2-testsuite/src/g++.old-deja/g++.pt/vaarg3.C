// { dg-do assemble  }
// Copyright (C) 2000 Free Software Foundation
// Contributed by Nathan Sidwell 22 June 2000 <nathan@codesourcery.com>

typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type) // { dg-error "" }

struct A {
  virtual ~A () {};
};

template <class Type>
void PrintArgs (Type somearg, ...)
{ 
va_list argp;
va_start (argp, somearg);
Type value;
value = va_arg (argp, Type); // { dg-error "" } cannot pass non-POD
va_end (argp);
}

int main (void)
{
A dummy;
PrintArgs (dummy, dummy); // { dg-error "" } cannot pass non-POD
return 0;
}
