/* APPLE LOCAL file 4492976 */
/* Test if constant CFStrings get placed in the correct section.  */

/* { dg-options "-fconstant-cfstrings -m64" } */
/* { dg-do compile { target *-*-darwin* } } */

typedef const struct __CFString * CFStringRef;
CFStringRef appKey = (CFStringRef) @"com.apple.soundpref";

/* { dg-final { scan-assembler ".section __DATA, __cfstring" } } */
/* { dg-final { scan-assembler ".quad\t___CFConstantStringClassReference\n\t.long\t1992\n\t.space 4\n\t.quad\t.*\n\t.quad\t19\n\t.data" } } */
