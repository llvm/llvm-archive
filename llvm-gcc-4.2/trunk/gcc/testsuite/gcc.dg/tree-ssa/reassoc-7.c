/* { dg-do compile } */ 
/* { dg-options "-O2 -fdump-tree-reassoc1" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */
int main(int a, int b, int c, int d, int e, int f, int g, int h)
{
  /* Should be transformed into a + c + d + e + g + 15 */
  int i = (a + 9) + (c + d);
  int j = (e + 4) + (2 + g);
  e = i + j;
  return e;
}
/* { dg-final { scan-tree-dump-times "\\\+ 15" 1 "reassoc1"} } */
/* { dg-final { cleanup-tree-dump "reassoc1" } } */
