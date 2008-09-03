/* APPLE LOCAL file radar 5928316 */
/* Test for use of block pointer in a conditional expression. */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

#include <stdio.h>
void * _NSConcreteStackBlock;

typedef int (^myblock)(int);

int main() {
        myblock b = ^(int a){ return a * a; };
        if (1 && (b)) {
                int i = b(3);
                printf("i = %d\n", i);
        }
        return 0;
}


