/* APPLE LOCAL file radar 4436866 */
/* This program checks for proper declaration of property. */
/* { dg-do compile { target *-*-darwin* } } */

@interface Bar
@end

@implementation Bar
@property int foo; /* { dg-error "no declaration of property \\'foo\\' found in the interface" } */
@end
