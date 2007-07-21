/* APPLE LOCAL file radar 4666559 */
/* { dg-options "-gdwarf-2 -dA" } */
/* { dg-final { scan-assembler "\"_prop\\\\0\".*DW_AT_name" } } */
@interface Foo 
{
  id isa;
}
@property (ivar) const char* prop;
@end

@implementation Foo 
@property(ivar) const char* prop;
@end
