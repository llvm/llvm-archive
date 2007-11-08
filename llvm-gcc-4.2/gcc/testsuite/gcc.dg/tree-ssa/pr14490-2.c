/* { dg-do compile } */
/* { dg-options "-fdump-tree-gimple -fwrapv" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */
int g(int x)
{
   return (x - 10) < 0;
}
/* There should be no x >= 9 and one x - 10. */
/* { dg-final { scan-tree-dump-times "<= 9" 0 "gimple"} } */
/* { dg-final { scan-tree-dump-times "\\+ -10" 1 "gimple"} } */
/* { dg-final { cleanup-tree-dump "gimple" } } */
