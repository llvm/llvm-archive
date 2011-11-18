/* APPLE LOCAL file radar 5732232 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

void donotwarn();

int (^IFP) ();
int (^II) (int);
int test1() {
  int (^PFR) (int) = 0;	// OK
  PFR = II;	// OK

  if (PFR == II)	// OK
    donotwarn();

  if (PFR == IFP)
    donotwarn();

  if (PFR == (int (^) (int))IFP) // OK
    donotwarn();

  if (PFR == 0) // OK
    donotwarn();

  if (PFR)	// OK
    donotwarn();

  if (!PFR)	// OK
    donotwarn();

  return PFR != IFP;
}

int test2(double (^S)()) {
  double (^I)(int)  = (void*) S;
  (void*)I = (void *)S;  /* { dg-warning "assignment to cast is illegal" } */

  void *pv = I;

  pv = S;		

  I(1);

  return (void*)I == (void *)S;
}

int^ x;  /* { dg-error "block pointer to non-function type is invalid" } */
int^^ x1; /* { dg-error "block pointer to non-function type is invalid" } */

int test3() {
  char *^ y;  /* { dg-error "block pointer to non-function type is invalid" } */
}
