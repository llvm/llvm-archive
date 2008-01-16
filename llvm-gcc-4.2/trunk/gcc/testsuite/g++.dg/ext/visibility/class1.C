// PR c++/26905
// Init should not be hidden, so calling it should use the PLT.

// { dg-options "-fpic" }
// { dg-do compile { target i?86-*-* x86_64-*-* *-*-darwin* } }
// APPLE LOCAL mainline candidate
// { dg-skip-if "" { { i?86-*-darwin* x86_64-*-darwin* } && lp64 } { "*" } { "" } }
// { dg-require-visibility "" }
// { dg-final { scan-assembler "InitEv@PLT|indirect_symbol.*InitEv" } }

#pragma GCC visibility push(hidden)
struct __attribute__ ((visibility ("default"))) nsINIParser
{
    static void Init();
};

__attribute__ ((visibility ("default")))
void
CheckCompatibility(void)
{
  nsINIParser::Init();
}
