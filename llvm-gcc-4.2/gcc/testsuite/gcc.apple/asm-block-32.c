/* APPLE LOCAL file CW asm blocks */
/* { dg-do assemble { target i?86*-*-darwin* } } */
/* { dg-skip-if "" { *-*-darwin* } { "-m64" } { "" } } */
/* { dg-options { -fasm-blocks -msse3 } } */
/* LLVM LOCAL disable test */
/* { dg-skip-if "" { *-*-darwin* } { "*" } { "" } } */
/* Radar 4319887 */

void bar();

void foo () {
  asm {
    call bar
    call foo
  }
}
