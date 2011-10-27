/* PR 23190 */
/* { dg-do compile } */
/* { dg-options "-gdwarf-2 -dA" } */
/* { dg-final { scan-assembler "\\.Lstring3\[^\\n\\r\]+DW_AT_name" } } */
/* { dg-final { scan-assembler "\\.Lstring3:\[\\n\\r \\t\]+\\.ascii\\s+.xyzzy." } } */

void f(void)
{
   static int xyzzy;
   xyzzy += 3;
}
