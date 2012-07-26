/* APPLE LOCAL file weak_import on property 6676828 */
/* Ignore weak_import on properties. */
/* Radar 6676828 */
/* { dg-do compile { target *-*-darwin* } } */

__attribute__((objc_root_class)) @interface foo
@property(nonatomic) int foo __attribute__((weak_import));
@end
