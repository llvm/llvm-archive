/* APPLE LOCAL file 5932809 */
/* { dg-options "-fblocks" } */
/* { dg-do run } */
void _Block_byref_release(void*src){}

#include <stdio.h>

int main() {
   __byref  int X = 1234;
   __byref  const char * message = "HELLO";

   X = X - 1234;

   X += 1;

   printf ("%s(%d)\n", message, X);
   X -= 1;

   return X;
}
