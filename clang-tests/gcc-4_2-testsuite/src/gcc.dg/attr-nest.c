/* { dg-do compile } */

#define ATTR_PRINTF __attribute__ ((format (printf, 1, 2))) /* { dg-warning "expanded from macro" } */
#define ATTR_USED __attribute__ ((used)) /* { dg-warning "expanded from macro" } */


void bar (int, ...);

/* gcc would segfault on the nested attribute.  */
void foo (void)
{
  bar (0, (void (*ATTR_PRINTF) (const char *, ...)) 0); /* { dg-error "attribute ignored" } */
}

/* For consistency, unnamed decls should give the same warnings as
   named ones.  */
void proto1 (int (*ATTR_USED) (void)); /* { dg-warning "attribute ignored" } */
void proto2 (int (*ATTR_USED bar) (void)); /* { dg-warning "attribute ignored" } */
