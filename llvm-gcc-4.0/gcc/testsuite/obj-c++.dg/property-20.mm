/* APPLE LOCAL file radar 4695274 */
/* Check for correct offset calculation of inserted 'ivar' into the interface class. */
/* { dg-do run { target *-*-darwin* } } */

#include <objc/objc.h>
#include <objc/Object.h>
#include <stdlib.h>

@interface BASE
{
	double ivarBASE;
}
@property (ivar) double pp;
@end

@implementation BASE
@end

@interface XXX : BASE
{
@public
	char *pch1;
}
@end

@implementation XXX
@end

int main ()
{
  if (offsetof (XXX, pch1) != 16)
    abort ();
  return 0;
}
