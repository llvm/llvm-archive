/* APPLE LOCAL file mainline */
// { dg-options "-W" }

#define NOPE

void foo() {
  while (1); /* { dg-error "while loop has empty body" }
                { dg-error "put the semicolon" "" { target *-*-* } 7 } */
    {
    }
  for (;;); /* { dg-error "for loop has empty body" 1 }
                { dg-error "put the semicolon" "" { target *-*-* } 11 } */
    {
    }
  while (1)
    ;
  for (;;)
    ;
  while (1) ;
  for (;;) ;
  while (1) NOPE;
  for (;;) NOPE;
  while (1)
    NOPE;
  for (;;)
    NOPE;
}
