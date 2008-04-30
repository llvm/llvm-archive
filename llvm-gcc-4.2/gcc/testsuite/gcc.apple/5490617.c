/* APPLE LOCAL file mainline candidate */
/* { dg-do compile } */
static char _my_endbss[1];
char *my_endbss_static = _my_endbss;
/* LLVM LOCAL */
/* { dg-final { scan-assembler ".lcomm\t__my_endbss" } } */
