/* APPLE LOCAL file radar 6177162 */
/* local statics need be treated same as file static (byref). */
/* { dg-options "-mmacosx-version-min=10.6" { target *-*-darwin* } } */
/* { dg-do run } */

#include <stdio.h>

int main(int argc, char **argv) {
  static int numberOfSquesals = 5;

  ^{ numberOfSquesals = 6; }();

  if (numberOfSquesals == 6) { printf("%s: success\n", argv[0]); }

  return 0;
}
