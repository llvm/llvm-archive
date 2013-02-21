// { dg-do assemble  }
// { dg-options "-Wnested-anon-types" }

static union {
  union {  // { dg-warning "" } types declared in an anonymous union
  };
};
