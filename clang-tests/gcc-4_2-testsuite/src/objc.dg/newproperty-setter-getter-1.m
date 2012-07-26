/* APPLE LOCAL file radar 4805321 */
/* { dg-options "-fobjc-new-property -mmacosx-version-min=10.5" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-options "-fobjc-new-property" { target arm*-*-darwin* } } */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface Bar 
@property (assign, setter = MySetter:) int FooBar;
- (void) MySetter : (int) value;
- (int) FooBar;
@property (assign, getter = MyGetter) int PropGetter;
- (int) MyGetter;
@property (assign) int noSetterGetterProp;
@end
