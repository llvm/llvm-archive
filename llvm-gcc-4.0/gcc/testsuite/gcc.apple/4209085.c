/* APPLE LOCAL file 4209085 */
/* Generate fisttp if SSE3 is enabled */
/* { dg-do compile { target i?86-*-darwin* } } */
/* { dg-options "-O2 -msse3" } */
double cos(double);
int  main(void) {
    return cos(1);
}
/* { dg-final { scan-assembler "fisttp" } } */
