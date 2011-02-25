// Copyright (C) 2005 Free Software Foundation, Inc.
// Contributed by Nathan Sidwell 13 Jun 2005 <nathan@codesourcery.com>

// Origin:  Volker Reichelt <reichelt@gcc.gnu.org>
// Bug 21929: ICE on invalid

template<int> struct A // { dg-error "here" }
{
    struct B;
};

template<> struct A<void>::B {}; // { dg-error "expression|extraneous" }
