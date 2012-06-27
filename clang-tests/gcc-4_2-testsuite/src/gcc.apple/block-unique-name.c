/* APPLE LOCAL file radar 6411649 */
/* { dg-do compile } */
/* { dg-options "-mmacosx-version-min=10.6 -fblocks" { target *-*-darwin* } } */

typedef void (^P) (void);

int DOO(void)
{
  P p = ^{ (void) ^ { ; }; };
  p();
  p = ^{ (void) ^ { ; }; };
  return 0;
}

int main(void)
{
  P p = ^{ ; };
  p();
  p = ^{ ; };
        return 0;
}
/* { dg-final { scan-assembler "___main_block_invoke" } } */
/* { dg-final { scan-assembler "___main_block_invoke_2" } } */
/* { dg-final { scan-assembler "___DOO_block_invoke" } } */
/* { dg-final { scan-assembler "___DOO_block_invoke_2" } } */


