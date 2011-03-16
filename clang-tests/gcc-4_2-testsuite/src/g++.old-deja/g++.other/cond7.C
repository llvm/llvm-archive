// { dg-do assemble  }
// 
// Copyright (C) 2001 Free Software Foundation, Inc.
// Contributed by Nathan Sidwell 24 Jul 2001 <nathan@codesourcery.com>

// Bug 3416. We left some unchecked overloaded functions lying around.

struct X
{
  void operator << (int);
  void operator << (float);
};

void OVL1 (int); // { dg-error "note" }
void OVL1 (float); // { dg-error "note" }

void OVL2 (int);
void OVL2 (float);

X x;

void foo (bool a)
{
  x << (a ? OVL1 : OVL2);	// { dg-error "" } incomplete type
  a ? OVL1 : OVL2;              // { dg-error "" } incomplete type
}
