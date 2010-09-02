/* PR c/9262 */
/* Originator: Rasmus Hahn <rassahah@neofonie.de> */
/* { dg-do compile } */

int foo(int i)
{	/* { dg-error "note" } */
  switch (i)
    case 3:
      return 1,
}  /* { dg-error "expected" } */
   /* { dg-error "expected" } */
