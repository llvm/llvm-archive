/* APPLE LOCAL file radar 4582997 */
/* Test that property need not be declared in @implementation for it to be used. */
/* { dg-options "-framework Cocoa" } */
/* { dg-do run { target *-*-darwin* } } */
#include <Foundation/Foundation.h>

@interface Foo : NSObject
{
@private
  int _userDefined;
}
@property (ivar) int bar;
@property int userDefined;
@end

@implementation Foo
@property int bar;

- (void)setUserDefined:(int)f
{
  _userDefined = f;
  if (self.userDefined != _userDefined)
    abort ();
}
- (int)userDefined
{
  return _userDefined;
}
@end

@interface MyObserver : NSObject 
@end

@implementation MyObserver

-(void)observeValueForKeyPath:(NSString*)keypath ofObject:obj change:(NSDictionary *)change context:(void *) context
{
  printf("observing %s is now %d\n", [keypath cString], ((Foo *)obj).bar);
}
@end

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	Foo *f = [[Foo alloc] init];
	[f addObserver:[[MyObserver alloc] init] forKeyPath:@"bar" options:NSKeyValueObservingOptionNew context:0];
	f.bar = 707;
	f.bar = 808;
	f.bar = 909;
	f.userDefined = 707;
	f.userDefined = 808;
	f.userDefined = 909;
    [pool release];
    return 0;
}

