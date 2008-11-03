/* APPLE LOCAL file radar 5932809 */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5 -ObjC++ -lobjc" { target *-*-darwin* } } */

#include <stdio.h>

void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}


int i;

int foo() {
   __block  id FFFFFF;
   __block  id Q;
   ^{ FFFFFF = 0; }; 

   if (i)
   {
     __block  id FFFFFF;
     __block  id Q;
     ^{ FFFFFF = 0; }; 
   }
}

int main() {
   __block  id X;
   __block  id X1;
   ^{  X = 0; }; 

   if (i)
   {
     __block  id X;
     __block  id X1;
     ^{ X = 0; }; 
   }
   return 0;
}


