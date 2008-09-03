/* APPLE LOCAL file radar 5732232 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

void (^test3())(void) {
  return ^{};   /* { dg-error "returning block that lives on the local stack" } */
}

