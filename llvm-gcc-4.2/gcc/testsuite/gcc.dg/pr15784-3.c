/* { dg-do compile } */
/* SH4 without -mieee defaults to -ffinite-math-only.  */
/* { dg-options "-fdump-tree-gimple -fno-finite-math-only" } */
/* LLVM LOCAL test not applicable */
/* { dg-require-fdump "" } */
/* Test for folding abs(x) where appropriate.  */
#define abs(x) x > 0 ? x : -x
extern double fabs (double);

int a (float x) {
	return fabs(x) >= 0.0;
}

/* { dg-final { scan-tree-dump-times "ABS_EXPR" 1 "gimple" } } */
/* { dg-final { cleanup-tree-dump "gimple" } } */
