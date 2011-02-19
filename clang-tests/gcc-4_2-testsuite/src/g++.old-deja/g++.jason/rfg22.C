// { dg-do assemble  }
static void f ();		// { dg-error "has internal linkage but is not defined" }

void g ()
{
  f (); /* { dg-error "note: used here" } */
}
