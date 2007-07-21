/* APPLE LOCAL file radar 4734562 */
/* { dg-options "-gdwarf-2 -dA" } */
/* { dg-final { scan-assembler "\"main\\\\0\".*DW_AT_name" } } */

#include <objc/objc.h>
#include <objc/Object.h>
extern "C" void abort (void);

@interface Bar : Object
{
  int iVar;
  int iBar;
  float f;
}
@end

@implementation Bar
- (void) MySetter : (int) value { iVar = value; }

@end

int main(int argc, char *argv[]) {
    Bar *f = [Bar new];
	return 0;
}

