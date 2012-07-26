/* APPLE LOCAL file radar 4899564 */
/* 'retain' or 'copy' are nonsensical in our system when used with __weak, 
    and should provoke an error. */
/* { dg-options "-fobjc-new-property -mmacosx-version-min=10.5 -fobjc-gc" } */
/* { dg-do compile { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-require-effective-target objc_gc } */

__attribute__((objc_root_class)) @interface DooFus  {
   __weak id y;
   __weak id x;
   __weak id z;
}
@property (assign) __weak id y;
@property (copy) __weak id x; /* { dg-error "property attributes 'copy' and 'weak' are mutually exclusive" } */
@property (retain) __weak id z; /* { dg-error "property attributes 'retain' and 'weak' are mutually exclusive" } */

@end
