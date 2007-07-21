/* APPLE LOCAL file radar 4954480 */
/* Check for illegal use of 'ivar' in objc2 abi */
/* { dg-options "-m64" } */
/* { dg-do compile { target *-*-darwin* } } */
@interface LKLayerTransaction
{
@public
  LKLayerTransaction *next;
}
@end

@interface LKLayer @end

@implementation LKLayer

int LKLayerFreeTransaction ()
{
	return __builtin_offsetof (LKLayerTransaction, next); /* { dg-error "Illegal reference to 'none-fragile' ivar" } */
}
@end
