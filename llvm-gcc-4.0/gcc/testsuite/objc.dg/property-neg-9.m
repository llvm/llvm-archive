/* APPLE LOCAL file 4724822 */
/* Attempt to access property of a forward decl. class. */

@class NSString;
void foo(NSString *param_0) {
        param_0.val; /* { dg-error "request for member \\'val\\' in something not a structure or union" } */
}

