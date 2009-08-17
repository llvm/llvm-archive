/* Test generation of mullhwu. on 405.  */
/* Origin: Joseph Myers <joseph@codesourcery.com> */
/* { dg-do compile } */
/* { dg-require-effective-target ilp32 } */
/* { dg-options "-O2 -mcpu=405" } */

/* { dg-final { scan-assembler "mullhwu\\. " } } */

unsigned int
f(unsigned int b, unsigned int c)
{
  unsigned int a = (unsigned short)b * (unsigned short)c;
  if (!a)
    return 10;
  return a;
}
