/* { dg-options "-Wall -funit-at-a-time -fgnu-runtime" } */
/* { dg-do compile }  */
/* APPLE LOCAL objc2 */
/* { dg-require-effective-target ilp32 } */
/* PR objc/27438, make sure that the decl produced by the front-end
   does not cause a warning to be produced. */

__attribute__((objc_root_class)) @interface NXConstantString
{
  void *isa;
  const char * const nxcsptr;
  const unsigned int nxcslen;
}
@end
NXConstantString *a =   @"NSInconsistentArchiveException"; /* { dg-bogus "defined but not used" } */


