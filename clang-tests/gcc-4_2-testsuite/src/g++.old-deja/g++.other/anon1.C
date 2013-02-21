// { dg-do assemble  }
// { dg-options "-Wno-nested-anon-types" }

static union {
  union {
  };
}; // { dg-warning "" } anonymous union with no members
