/* { dg-do compile } */
/* { dg-options "-fdump-tree-gimple -fwrapv" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */
#define ABS(x) (x > 0 ? x : -x)
int f (int a) {
	return ABS (ABS(a));
}

/* { dg-final { scan-tree-dump-times "ABS" 1 "gimple" } } */
/* { dg-final { cleanup-tree-dump "gimple" } } */
