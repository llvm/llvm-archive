/* Test diagnostics for duplicate typedefs.  Basic diagnostics.  */
/* Origin: Joseph Myers <joseph@codesourcery.com> */
/* { dg-do compile } */
/* { dg-options "" } */

typedef int I; /* { dg-warning "previous definition" } */
typedef int I; /* { dg-warning "warning: redefinition of typedef 'I' is a C11 feature" } */

typedef int I1; /* { dg-error "error: previous declaration of 'I1' was here" } */
typedef long I1; /* { dg-error "error: conflicting types for 'I1'" } */
