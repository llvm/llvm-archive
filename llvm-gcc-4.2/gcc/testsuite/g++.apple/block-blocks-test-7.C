/* APPLE LOCAL file radar 5932809 */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5 -ObjC++ -lobjc" { target *-*-darwin* } } */

#include <stdio.h>

void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}


int i;

int foo() {
   __byref  id FFFFFF;
   __byref  id Q;
   ^{ FFFFFF = 0; }; 

   if (i)
   {
     __byref  id FFFFFF;
     __byref  id Q;
     ^{ FFFFFF = 0; }; 
   }
}

int main() {
   __byref  id X;
   __byref  id X1;
   ^{  X = 0; }; 

   if (i)
   {
     __byref  id X;
     __byref  id X1;
     ^{ X = 0; }; 
   }
   return 0;
}


