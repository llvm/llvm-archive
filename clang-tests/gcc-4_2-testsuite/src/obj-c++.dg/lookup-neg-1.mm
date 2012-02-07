/* APPLE LOCAL file radar 4829851 */
/* Test that objective-c++ compiler catches duplicate declarations in objective-c++'s 
   global namespace. */
@class One;  /* { dg-warning "previous definition is here" } */
int One;     /* { dg-error "redefinition of .One. as different kind of symbol" } */

@class Foo;  /* { dg-warning "previous definition is here" } */

namespace Foo { int x; } /* { dg-error "redefinition of .Foo. as different kind of symbol" } */

@class X;    /* { dg-warning "previous definition is here" } */

struct X {/* { dg-error "redefinition of .X. as different kind of symbol" } */
    int X;
};
