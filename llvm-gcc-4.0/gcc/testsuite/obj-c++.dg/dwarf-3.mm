/* APPLE LOCAL file radar 4477797 */
/* { dg-options "-gdwarf-2 -dA" } */
/* { dg-final { scan-assembler "\"Object\\\\0\".*DW_AT_name" } } */
/* { dg-final { scan-assembler "\"MyObject\\\\0\".*DW_AT_name.*DW_TAG_inheritance" } } */
#include <objc/objc.h>
#include <objc/Object.h>

@interface MyObject : Object {
  int myData;
}
@end

@implementation MyObject @end

int main()
{
  MyObject *obj = [[MyObject alloc] init];
}	
