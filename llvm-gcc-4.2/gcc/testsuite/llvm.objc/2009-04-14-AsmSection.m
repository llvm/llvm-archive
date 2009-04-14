/* LLVM LOCAL file rdar://6763960 */
/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-O0 -fobjc-abi-version=2" } */

@interface A
@end

@implementation A
@end

/* { dg-final { scan-assembler "OBJC_CLASS_$_A.*section.*__DATA, __objc_data" } } */
