/* APPLE LOCAL file 64bit shorten warning 3865314 */
/* { dg-do compile } */
/* { dg-options "-Wshorten-64-to-32" } */
/* Radar 3865314 */

long long ll;
int i;
char c;

void bar (int);

void foo() {
  c = i;
  c = ll;
  i = (int) ll;
  i = ll;	/* { dg-warning "-Wshorten-64-to-32" } */
  i += ll;	/* { dg-warning "-Wshorten-64-to-32" "" { xfail *-*-* } } <rdar://problem/10466193> */
  i = i ? ll : i;/* { dg-warning "-Wshorten-64-to-32" }*/
  bar (ll);	/* { dg-warning "-Wshorten-64-to-32" } */
}
