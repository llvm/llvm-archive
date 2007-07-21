/* APPLE LOCAL file radar 4727191 */
/* Test that assigning local variable to a property does not ICE. */
/* { dg-do compile { target *-*-darwin* } } */

@interface Link 
  @property(ivar) id test;
@end

@implementation Link @end

int main() {
    id pid;
    Link *link;
    link.test = pid;
}
