/* { dg-do compile } */
/* { dg-options "-O1 -fdump-tree-ccp-vops" } */

/* Test to check whether global variables are being
   constant propagated. */

int G;

foo (int i)
{
   if (i > 0)
     G = 3;
   else
     G = 3;

   if (G != 3)
     link_error ();
}

main ()
{
   foo (0);
   return 0;
}

/* There should be no G on the RHS of an assignment. */
/* { dg-final { scan-tree-dump-times "= G;" 0 "ccp"} } */
