/* PR c/5597 */
/* { dg-do compile } */
/* { dg-options "" } */

/* Verify that GCC forbids non-static initialization of
   flexible array members. */

struct str { int len; char s[]; }; /* { dg-warning "initialized" } */

struct str a = { 2, "a" };

void foo()
{
  static struct str b = { 2, "b" };
  struct str c = { 2, "c" }; /* { dg-error "initialization of flexible array member" } */
  struct str d = (struct str) { 2, "d" }; /* { dg-error "initialization of flexible array member" } */
  struct str e = (struct str) { d.len, "e" }; /* { dg-error "initialization of flexible array member" } */
}
