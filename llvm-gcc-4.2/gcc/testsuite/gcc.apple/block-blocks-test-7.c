/* APPLE LOCAL file radar 5932809 */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5 -ObjC -lobjc" { target *-*-darwin* } } */

#include <stdio.h>

void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}


int i;

int foo() {
   __byref  id FFFFFF;
   __byref  id Q;
   ^{ |FFFFFF| FFFFFF = 0; }; /* { dg-warning "has been deprecated in blocks" } */

   if (i)
   {
     __byref  id FFFFFF;
     __byref  id Q;
     ^{ |FFFFFF| FFFFFF = 0; }; /* { dg-warning "has been deprecated in blocks" } */
   }
}

int main() {
   __byref  id X;
   __byref  id X1;
   ^{ |X| X = 0; }; /* { dg-warning "has been deprecated in blocks" } */

   if (i)
   {
     __byref  id X;
     __byref  id X1;
     ^{ |X| X = 0; }; /* { dg-warning "has been deprecated in blocks" } */
   }
   return 0;
}


