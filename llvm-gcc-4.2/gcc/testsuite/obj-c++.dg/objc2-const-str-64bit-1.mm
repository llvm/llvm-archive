/* APPLE LOCAL file 4719165 */
/* Test for new way of representing constant string structure */
/* { dg-options "-fnext-runtime -m64 -fobjc-abi-version=2" } */
/* { dg-do compile { target powerpc-*-darwin* } } */

@interface NSConstantString { id isa; const char *c; int l; } @end
@implementation NSConstantString @end

int main() {
    return (int)(long)@"foo";
}
/* { dg-final { scan-assembler "LC1:\n\t.quad\t_OBJC_CLASS_\\\$_NSConstantString" } } */
