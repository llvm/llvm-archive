/* APPLE LOCAL begin radar 4278236 */
class TestCPP { }; /* { dg-warning "previous definition is here" } */

@class TestCPP;    /* { dg-error "redefinition of .TestCPP. as different kind of symbol" } */
/* APPLE LOCAL end radar 4278236 */
