/* APPLE LOCAL file radar 6014138 */
/* Test use of __byref on locals which will be used as 'byref' variables in blocks. */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5 -fblocks" { target *-*-darwin* } } */

#include <stdio.h>
void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}

void CallBlock (void (^voidvoidBlock)(void)) {
   voidvoidBlock();
}


int foo() {
    __byref int local_byref_1 = 10;
    __byref int local_byref_2 = 10;
    __byref int local_byref_3 = 10;

    CallBlock( ^ { ++local_byref_1; ++local_byref_2; ++local_byref_3; 
	          CallBlock(^ { ++local_byref_1; ++local_byref_2; ++local_byref_3; }); 
		  ++local_byref_1; ++local_byref_2; ++local_byref_3; });

    if (local_byref_1 != 13 || local_byref_2 != 13 || local_byref_3 != 13)
      return 1;
    return 0;
}

int main()
{
  return foo ();
}

