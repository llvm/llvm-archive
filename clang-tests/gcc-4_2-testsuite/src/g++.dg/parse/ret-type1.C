// PR c++/2738
// Origin: Wolfgang Bangerth <wolfgang.bangerth@iwr.uni-heidelberg.de>
// { dg-do compile }

template <int i> class A
{
    class C { C(); };
};

template <int i> void A<i>::C::C () {}  // { dg-error "member 'C' has the same name as its class" }
					// { dg-error "constructor cannot have a return type" "" { target *-*-* } 10 }
