/* APPLE LOCAL file radar 5732232 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

typedef void (^CL)(void);

CL foo() {
  short y;
  short (^add1)(void) = ^{ return y+1; };  /* { dg-error "incompatible block pointer types initializing" } */

  CL X = ^{
    if (2)
      return;
    return 1;		/* { dg-error "return type 'int' must match previous return type 'void' when block literal has unspecified explicit return type" } */
  };

  int (^Y) (void)  = ^{
    if (3)
      return 1;
    else
      return;		/* { dg-error "return type 'void' must match previous return type 'int' when block literal has unspecified explicit return type" } */
  };

  char *(^Z)(void) = ^{
    if (3)
      return "";
    else
      return (char*)0;
  };

  double (^A)(void) = ^ { /* { dg-error "incompatible block pointer types initializing" } */
    if (1)
      return (float)1.0;
    else
      if (2)
	return (double)2.0;	/* { dg-error "return type 'double' must match previous return type 'float' when block literal has unspecified explicit return type" } */
    return 1;		/* { dg-error "return type 'int' must match previous return type 'float' when block literal has unspecified explicit return type" } */
  };	
  char *(^B)(void) = ^{
    if (3)
      return "";
    else
      return 2;		/* { dg-error "return type" } */
  };

  return ^{ return 1; };	/* { dg-error "incompatible block pointer types returning" } */
}
