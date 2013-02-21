// { dg-do assemble  }
#include <stdio.h>

template <int n1>
double val <int> () // { dg-error "" } bogus code
                    // { dg-error "" "" { target *-*-* } 5 } bogus code
                    // { dg-error "" "" { target *-*-* } 5 } bogus code
{                          
   return (double) n1;
}

int main ()
{
   printf ("%d\n", val<(int)3> ()); // { dg-error "" } val undeclared
                    		    // { dg-error "" "" { target *-*-* } 14 } bogus code
}
