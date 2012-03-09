/* Test for scanf formats.  Formats using extensions to the standard
   should be rejected in strict pedantic mode.
*/
/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do compile } */
/* { dg-options "-std=iso9899:1990 -pedantic -Wformat" } */

#include "format.h"

void
foo (char **sp, wchar_t **lsp)
{
  /* %a formats for allocation, only recognized in C90 mode, are a
     GNU extension.
  */
  scanf ("%as", sp); /* { dg-warning "not supported" "%as" } */
  scanf ("%aS", lsp); /* { dg-warning "not supported" "%aS" } */
  scanf ("%a[bcd]", sp); /* { dg-warning "not supported" "%a[]" } */
}
