/* APPLE LOCAL file radar 6212722 */
/* APPLE LOCAL radar 7721728 */
/* This is now error */
/* Test for use of array (dynamic or static) as copied in object in a block. */
/* { dg-do compile { target *-*-darwin[1-2][0-9]* } } */
/* { dg-options "-mmacosx-version-min=10.6 -ObjC -framework Foundation" { target *-*-darwin* } } */
/* { dg-skip-if "" { powerpc*-*-darwin* } { "-m64" } { "" } } */

#import <Foundation/Foundation.h>
#import <Block.h>


int _getArrayCount() {return 5;}


int func ()
{
	NSAutoreleasePool *pool	= [[NSAutoreleasePool alloc] init];

	int array[5]; /* { dg-error "declared here" } */
	
	int i;
	const int c = 5;
	for (i = 0; i < c; ++i)
	{
		array[i] = i+1;
	}
	
	void (^block)(void) = ^{
	
		int i;
		NSLog (@"c = %d", c);
		for (i = 0; i < c; ++i)
		{
			NSLog (@"array[%d] = %d", i, array[i]);	/* { dg-error "cannot refer to declaration with an array type inside block" } */
		}
	
	};
	
	block();

	[pool drain];
	return 0;
}

int main (int argc, const char *argv[])
{
        int res;
	NSAutoreleasePool *pool	= [[NSAutoreleasePool alloc] init];

	int array[_getArrayCount()]; /* { dg-error "declared here" } */
	
	int i;
	const int c = _getArrayCount();
	for (i = 0; i < c; ++i)
	{
		array[i] = i+1;
	}
	
	void (^block)(void) = ^{
	
		int i;
		//const int c = _getArrayCount();
		NSLog (@"c = %d", c);
		for (i = 0; i < c; ++i)
		{
			NSLog (@"array[%d] = %d", i, array[i]);	/* { dg-error "cannot refer to declaration with a variably modified type inside block" } */
		}
	
	};
	
	block();
	res = func();

	[pool drain];
	return 0 + res;
}

int test()
{
__block int arr[100]; /* { dg-error "declared here" } */

  ^ {  
      (void)arr[2];	/* { dg-error "cannot refer to declaration with an array type inside block" } */
    };
}
