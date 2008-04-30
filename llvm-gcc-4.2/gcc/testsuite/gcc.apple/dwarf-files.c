/* APPLE LOCAL file  */
/* Radar 5645155 */
/* { dg-do compile } */
/* { dg-options "-c -g -fasm-blocks" } */
/* { dg-final { scan-assembler-times "dwarf-files.c" 2 } } */
/* LLVM LOCAL disable test */
/* { dg-skip-if "" { *-*-darwin* } { "*" } { "" } } */
asm(".globl _x\n"
    "_x:\n"
    "pushl      %ebp\n");
