/* APPLE LOCAL file objc gc 5547128 */
/* { dg-do compile } */
/* { dg-options "-fobjc-gc" } */
/* Radar 5547128 */

__strong float *fp;

void foo() {
  fp[0] = 3.14;
}
