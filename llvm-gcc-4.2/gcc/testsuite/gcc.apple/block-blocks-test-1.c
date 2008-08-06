/* APPLE LOCAL file 5932809 */
/* { dg-options "-fblocks" } */
/* { dg-do compile } */

__byref  int X; /* { dg-warning "__byref attribute is only allowed on local variables - ignored" } */

int foo(__byref int param) { /* { dg-warning "byref attribute can be specified on variables only - ignored" } */
  __byref int OK = 1;

  extern __byref double extern_var;	/* { dg-warning "__byref attribute is only allowed on local variables - ignored" } */
  if (X) {
	static __byref char * pch;	/* { dg-warning "__byref attribute is only allowed on local variables - ignored" } */
  }
  return OK - 1;
}
