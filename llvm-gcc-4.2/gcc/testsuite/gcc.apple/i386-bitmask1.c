/* APPLE LOCAL file radar 4645709 */
/* { dg-do compile { target "i?86-*-*" } } */
/* { dg-options "-O2" } */
/* { dg-require-effective-target ilp32 } */
/* LLVM LOCAL */
/* { dg-final { scan-assembler "and.*(0xffffff00|4294967040)" } } */
unsigned char lut[256];
/* LLVM LOCAL make these global */
unsigned int *srcptr, *dstptr;

void foo( int count )
{
  int j;
	
  /* LLVM LOCAL begin remove uninitialized srcptr, dstptr */
  /* LLVM LOCAL end */
  for (j = 0; j < count; j++) {
    unsigned int tmp = *srcptr;
    unsigned int alpha = (tmp&255);
    tmp &= 0xffffff00;
    alpha =lut[alpha];
    tmp |= alpha<<0;
    *dstptr = tmp;
  }
}
/* APPLE LOCAL file radar 4645709 */

