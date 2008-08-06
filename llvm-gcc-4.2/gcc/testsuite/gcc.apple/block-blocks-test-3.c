/* APPLE LOCAL file 5932809 */
/* { dg-options "-fblocks" } */
/* { dg-do run } */

#include <stdio.h>
void _Block_byref_release(void*src){}

int main() {
   __byref  int X = 1234;
   __byref  const char * message = "HELLO\n";

   X = X - 1234;

   printf ("%s\n", message);
   return X;
}
