/* APPLE LOCAL file radar 5831920 */
#import <Foundation/Foundation.h>
/* Test a property with block type. */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5 -ObjC -framework Foundation" { target *-*-darwin* } } */

void * _NSConcreteStackBlock;
@interface TestObject : NSObject {

int (^getIntCopy)(void);

int (^getIntRetain)(void);

}
@property int (^getIntCopy)(void);
@property int (^getIntRetain)(void);
@end

@implementation TestObject
@synthesize getIntCopy;
@synthesize getIntRetain;

@end

int DoBlock (int (^getIntCopy)(void))
{
  return getIntCopy();
}



int main(char *argc, char *argv[]) {
    int count;
    int val = 0;
    TestObject *to = [[TestObject alloc] init];
    to.getIntRetain = ^ { | val| printf("\n Hello(%d)\n", val); return ++val; }; /* { dg-warning "has been deprecated in blocks" } */
    to.getIntCopy = to.getIntRetain;

    count = DoBlock (to.getIntCopy);
    if (count != 1)
      abort();
    count = DoBlock (to.getIntRetain);
    count = DoBlock (to.getIntRetain);
    return count - 3;
}
