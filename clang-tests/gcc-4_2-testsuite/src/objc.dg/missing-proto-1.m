/* Test for graceful handling of missing protocol declarations.  */
/* Author: Ziemowit Laski <zlaski@apple.com>.  */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface Foo <Missing> /* { dg-error "cannot find protocol declaration for .Missing." } */
@end
