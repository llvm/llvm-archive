/* APPLE LOCAL file radar 5732232, 6034839 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

void foo(int x, int y);

int main() {
  ^(int x)foo(x, 4); /* { dg-error "expected expression" } */
  ^(int x, int y)foo(y, x); /* { dg-error "expected expression" } */
  ^(int x)(x+4); /* { dg-error "expected expression" }
                    { dg-error "use of undeclared" "" { target *-*-* } 10 } */
}

