/* APPLE LOCAL file radar 4738176 */
/* Test that no bogus warning is issued in the synthesize compound-expression. */
/* { dg-options "-Wall" } */
/* { dg-do compile } */

@interface test
@property(ivar) int foo;
@property(ivar) int foo1;
@property(ivar) int foo2;
@end
extern int one ();
extern int two ();

@implementation test
- (void) pickWithWarning:(int)which { 
	   self.foo = (which ? 1 : 2); 
	   self.foo1 = self.foo2 = (which ? 1 : 2); 
	   self.foo = (which ? one() : two() ); 
	   self.foo1 = self.foo2 = (which ? one() : two ()); 
}
@end
