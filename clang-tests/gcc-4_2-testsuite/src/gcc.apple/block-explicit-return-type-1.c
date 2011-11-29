/* APPLE LOCAL file blocks 6339747 */
/* Test for blocks with explicit return type specified. */
/* { dg-do compile } */
/* { dg-options "-mmacosx-version-min=10.6" { target *-*-darwin* } } */

int (*funcptr)(long);
int sz1 = sizeof (int (*(^)(char x))(long));
int sz2 = sizeof (^int(*)(int p1) { return 0; });

int (*fun(int p1))(double) {return 0;  }

void (^b1)() = ^{ };
int (^b2)() = ^{ return 0; };
int (^b3)() = ^int { return 0; };
int (^b4)() = ^() { return 0; };
int (^b5)(char*) = ^(char *p1) { return 0; };
int (^b6)() = ^int () { return 0; };
int (^b7)(char) = ^int (char p1) { return 0; };
int (*(^b8)())(long) = ^int (*)(long) {return funcptr;};
int (*(^b9)(char))(long) = ^int (*(char x))(long) {return funcptr;};
void (^b10)(int) = ^(int p1){ };
int (*(*funcptr2)(long))[5];
int (*(*(^b11)(char))(long))[5] =
  ^int (*(*(char x))(long))[5] { if (x) x='a'; return funcptr2;};

int sz4 = sizeof (^int(*)(int p1)(int) { return 0; }); /* { dg-error "function cannot return function type" } */

int (*funcptr3[5])(long);
int (*funcptr4)(long);
int sz5 = sizeof (^(int i) { return i; });
int sz6 = sizeof (^int (int i) { return i; });
int sz7 = sizeof(^int (*())(long) {return funcptr4;});
int sz8 = sizeof(^int (*[5])(long) {return (int)funcptr3;});	/* { dg-error " block cannot return array type" } */
int sz9 = sizeof (^int (*((*)[5]))(long j) { j; });	/* { dg-error "undeclared" } */
