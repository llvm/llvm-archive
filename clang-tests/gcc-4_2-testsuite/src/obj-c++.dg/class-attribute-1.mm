/* APPLE LOCAL file 4548636 */
/* Check for a variety of rules for objc's class attributes. */
/* APPLE LOCAL radar 4899595 */
/* { dg-options "-Wno-objc-root-class -mmacosx-version-min=10.5" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-do compile } */

#include <objc/objc.h>
/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"

__attribute ((deprecated))  
@interface DEPRECATED : Object /* { dg-warning "declared here" "" { target *-*-* } } */
  { @public int ivar; } 
  - (int) instancemethod;
@property int prop; 
@end

@implementation DEPRECATED
@dynamic prop;
  -(int) instancemethod {  return ivar; } 
@end

__attribute ((deprecated)) void DEP(); /* { dg-warning "declared here" } */

@interface DEPRECATED (Category)
@end

@interface NS : DEPRECATED /* { dg-warning "deprecated" } */
@end

__attribute ((unavailable)) __attribute ((deprecated)) __attribute ((XXXX))  /* { dg-warning "unknown" } */
@interface UNAVAILABLE /* { dg-warning "declaration has been explicitly marked unavailable here" } */
  - (int *) AnaotherInst;
  + (DEPRECATED*) return_deprecated; /* { dg-warning "deprecated" } */
  - (UNAVAILABLE *) return_unavailable; 
@end

DEPRECATED * deprecated_obj; /* { dg-warning "deprecated" } */

UNAVAILABLE *unavailable_obj;		/* { dg-error "unavailable" } */

@implementation UNAVAILABLE
  - (int *) AnaotherInst { return (int*)0; }
  + (DEPRECATED *) return_deprecated { return deprecated_obj; } /* { dg-warning "deprecated" } */
  - (UNAVAILABLE *) return_unavailable { return unavailable_obj; }	/* { dg-error "unavailable" } */
@end

int foo (DEPRECATED *unavailable_obj) /* { dg-warning "deprecated" } */
{
    DEPRECATED *p =  /* { dg-warning "deprecated" } */
      [DEPRECATED new];	/* { dg-warning "deprecated" } */

    int ppp = p.prop;		
    p.prop = 1;
    (void)(p.prop != 3);		
    DEP();	/* { dg-warning "deprecated" } */
    int q = p->ivar;
    return [p instancemethod]; 
   
}
