/* APPLE LOCAL file radar 5698469 */
/* Check that warning includes use of word 'default' on missing assign attribute. */
/* { dg-options "-fobjc-gc -mmacosx-version-min=10.5" } */
/* { dg-do compile { target *-*-darwin* } } */

#include <Foundation/Foundation.h>

__attribute__((objc_root_class)) @interface INTF
@property NSDictionary* undoAction; /* { dg-warning "no 'assign', 'retain'" } */  
				    /* { dg-warning "default 'assign' attribute" "" { target *-*-* } 9 } */
@end

