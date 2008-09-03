/* APPLE LOCAL file radar 6040305 */
/* Test that we can run a simple block literal. */
/* { dg-do run } */
/* { dg-options "-fblocks" } */

#include <stdio.h>

void* _NSConcreteStackBlock;

int main()
{
    void (^xxx)(void) = ^{ printf("Hello world!\n"); };
    
    xxx();
    return 0;
}

