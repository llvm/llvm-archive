/* Regression test for paste corner cases.  Distilled from
   syscall stub logic in glibc.  */

/* { dg-do compile } */

#define ENTRY(name)	name##: 
/* { dg-warning "expanded from" "" { target *-*-* } 6 } */
#define socket bind

int
main(void)
{
  goto socket;

  ENTRY(socket) /* { dg-warning "valid preprocessing token" "" } */
    return 0;
}
