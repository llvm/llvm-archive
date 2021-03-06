/* { dg-do compile } */
/* { dg-options "-O1 -fdump-tree-optimized" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */

struct A
{
  int i : 8;
};

signed char c1, c2;
struct A a;

int main()
{
  a.i = c1;
  c2 = a.i;
  return a.i;
}

/* We should only store to a.i, not load from it.  */
/* { dg-final { scan-tree-dump-times "a.i" 1 "optimized" } } */
/* { dg-final { cleanup-tree-dump "optimized" } } */
