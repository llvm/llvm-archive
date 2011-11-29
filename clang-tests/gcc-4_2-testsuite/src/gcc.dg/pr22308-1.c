/* Bug 22308: C_TYPE_FIELDS_READONLY not updated on type variants.  */
/* { dg-do compile } */
/* { dg-options "" } */

struct foo s;
volatile struct foo t;
struct foo { const int z; };

void
bar (void)
{
  t = s; /* { dg-error "read-only variable is not assignable" } */
}
