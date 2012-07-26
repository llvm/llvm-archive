/* APPLE LOCAL file radar 4954480 */
/* Check for illegal use of 'ivar' in objc2 abi */
/* { dg-options "-mmacosx-version-min=10.5 -m64" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-do compile { target *-*-darwin* } } */
__attribute__((objc_root_class)) @interface LKLayerTransaction
{
@public
  LKLayerTransaction *next;
}
@end

__attribute__((objc_root_class)) @interface LKLayer @end

@implementation LKLayer

int LKLayerFreeTransaction ()
{
	return __builtin_offsetof (LKLayerTransaction, next); /* { dg-error "Illegal reference to non-fragile ivar" } */
}
@end
