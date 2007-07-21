/* Test default cpu type */
/* { dg-do compile { target powerpc*-*-darwin* } } */
/* { dg-options "-mmacosx-version-min=10.5" } */
int main()
{
  return 0;
}

/* { dg-final { scan-assembler "machine ppc7400" } } */

