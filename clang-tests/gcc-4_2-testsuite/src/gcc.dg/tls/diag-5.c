/* __thread specifiers on empty declarations.  */
/* { dg-require-effective-target tls } */

__thread struct foo; /* { dg-warning "ignored on this declaration" } */
