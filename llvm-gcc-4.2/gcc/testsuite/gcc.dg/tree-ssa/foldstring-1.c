/* { dg-do compile } */
/* { dg-options "-O1 -fdump-tree-useless" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */

void
arf ()
{
  if (""[0] == 0)
    blah ();
}
/* { dg-final { scan-tree-dump-times "= 0;" 1 "useless"} } */ 
/* { dg-final { cleanup-tree-dump "useless" } } */
