/* APPLE LOCAL file 4209014 4210936 */
/* { dg-do compile } */
/* { dg-options "-Os" } */
/* Call to inlinex1 should be inlined.  */
/* { dg-final { scan-assembler-not "(\tcall|\tbl)\[ 	a-zA-Z0-9_\]*inlinex1" } } */
/* Call to calleex1 should be called, not inlined.  */
/* { dg-final { scan-assembler "(\tcall|\tbl)\[ 	a-zA-Z0-9_\]*calleex1" } } */

/* Insure that trivial callees (up to 30 "estimated insns") are
   inlined only if marked 'inline' in C++/Obj-C++.  It's unfortunate,
   but this testcase is expected to require revision every time the
   GCC inlining heuristics change.  */

/* CALLs with three int arguments and no return type are assumed to generate 5
   'estimated insns'. */
extern void bulk (int, int, int);

/* Inlining limits for inline and non-inline functions are currently
   identical for C++.  Empirically, a body with few instructions will be
   inlined, and a body with several calls will not.  */

void
calleex1 (void)
{
  bulk (1,1,1); bulk (2,1,1); bulk (3,1,1); bulk (4,1,1);
  bulk (5,1,1); bulk (6,1,1);
}

void
inlinex1 (void)
{
  bulk (1,4,2);
}

int
main ()
{
  calleex1 ();
  inlinex1 ();
  return 0;
}
