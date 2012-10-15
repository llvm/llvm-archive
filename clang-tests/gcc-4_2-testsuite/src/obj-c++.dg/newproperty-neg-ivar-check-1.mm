/* APPLE LOCAL file radar 4816280 */
/* Diagnose as needed when 'ivar' synthesis is needed and it is not allowed. 
   'fragile' ivar (32bit abi) only. */
/* { dg-options "-fobjc-new-property -mmacosx-version-min=10.5 -fobjc-abi-version=1" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-options "-fobjc-new-property -fobjc-abi-version=1" { target arm*-*-darwin* } } */
/* { dg-do compile } */

@interface Moe /* { dg-warning "class \\'Moe\\' defined without specifying a base class" } */
@property int ivar; /* { dg-warning "setter and getter must both be synthesized, or both be user defined,or the property must be nonatomic" } */
@end
/* { dg-warning "add a super class to fix this problem"  "" { target *-*-* } 8} */
@implementation Moe
@synthesize ivar; /* { dg-error "synthesized property \\'ivar\\' must either be named the same as a compatible instance variable or must explicitly name an instance variable" } */
- (void)setIvar:(int)arg{} /* { dg-warning "writable atomic property 'ivar' cannot pair a synthesized getter with a user defined setter" } */
@end 

@interface Fred /* { dg-warning "class \\'Fred\\' defined without specifying a base class" } */
@property int ivar;
@end
/* { dg-warning "add a super class to fix this problem"  "" { target *-*-* } 17} */
@implementation Fred
// due to change to ivar spec, a @synthesize triggers an 'ivar' synthsis im 64bit 
// mode if one not found. In 32bit mode, lookup fails to find one and this result in an error.
// This is regardless of existance of setter/getters by user.
@synthesize ivar; /* { dg-error "synthesized property \\'ivar\\' must either be named the same as a compatible instance variable or must explicitly name an instance variable" } */
- (void)setIvar:(int)arg{}
- (int)ivar{return 1;}
@end

@interface Bob  /* { dg-warning "class \\'Bob\\' defined without specifying a base class" } */
@property int ivar;
@end
/* { dg-warning "add a super class to fix this problem"  "" { target *-*-* } 30} */
@implementation Bob
// no warning
@dynamic ivar;
- (int)ivar{return 1;}
@end

@interface Jade  /* { dg-warning "class \\'Jade\\' defined without specifying a base class" } */
@property int ivar;
@end
/* { dg-warning "add a super class to fix this problem"  "" { target *-*-* } 40} */
@implementation Jade
// no warning
- (void)setIvar:(int)arg{}
- (int)ivar{return 1;}
@end

int main (void) {return 0;}

