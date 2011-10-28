/* PR 23190 */
/* { dg-do compile } */
/* { dg-options "-gdwarf-2 -dA" } */
/* { dg-final { scan-assembler "\\.Lstring3\[ \\t\]+DW_AT_name" } } */
/* { dg-final { scan-assembler "\\.Lstring3:\n\[ \\t\]*\\.ascii\[ \\t\]+\"xyzzy\"" } } */

void f(void)
{
   static int xyzzy;
   xyzzy += 3;
}
