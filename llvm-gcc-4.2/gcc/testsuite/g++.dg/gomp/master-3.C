/* { dg-do compile } */
/* { dg-options "-fopenmp -fdump-tree-omplower" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */

extern void bar(int);

void foo (void)
{
  #pragma omp master
    bar(0);
}

/* { dg-final { scan-tree-dump-times "omp_get_thread_num" 1 "omplower" } } */
/* { dg-final { cleanup-tree-dump "omplower" } } */
