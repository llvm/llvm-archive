// { dg-do assemble  }
// { dg-options "" }

struct S 
{
  S ();  /* { dg-error "note" } */
};

union U {
  struct { 
    S s; /* { dg-error "anonymous struct member" } */
  };
};

