/* APPLE LOCAL file radar 5732232, 6034839, 6230297 - blocks */
/* { dg-do compile } */
/* { dg-options "-fblocks" } */

void I( void (^)(void));
void (^noop)(void);

void nothing();
int printf(const char*, ...);

typedef void (^T) (void);

void takeblock(T);
int takeintint(int (^C)(int)) { return C(4); }

T somefunction() {
  if (^{ })
    nothing();

  noop = ^{};

  noop = ^{printf("\nBlock\n"); };

  I(^{ });

  noop = ^noop /* { dg-error "requires a specifier" } */ 
	       /*  { dg-error "expected expression" "" { target *-*-* } 26 } */
    ;

  return ^{printf("\nBlock\n"); };  
}

void test2() {
  int x = 4;

  takeblock(^{ printf("%d\n", x); });
  takeblock(^{ x = 4; });  /* { dg-error "variable is not assignable" } */

  takeblock(^test2() /* { dg-error "requires a specifier" } */  
		     /* { dg-error "expected expression" "" { target *-*-* } 39 } */
	    );

  takeblock(^(void)(void)printf("hello world!\n")); /* { dg-error "expected expression" } */
}

void (^test3())(void) {
  /* APPLE LOCAL radar 6230297 */
  return ^{};    
}

void test4() {
  void (^noop)(void) = ^{};
  void (*noop2)() = 0;
}

void test5() {
  takeintint(^(int x)(x+1)); /* { dg-error "expected expression" }
                                { dg-error "use of undeclared identifier 'x'" "" { target *-*-* } 57 } */
  // Block expr of statement expr.
  takeintint(^(int x)({ /* { dg-error "expected expression" } */
			return 42; }));

  int y;
  takeintint(^(int x)(x+y)); /* { dg-error "expected expression" }
                                { dg-error "use of undeclared identifier 'x'" "" { target *-*-* } 64 } */

  void *X = ^(x+r);  /* { dg-error "expected" }
                        { dg-error "to match this" "" { target *-*-* } 67 } 
                        { dg-warning "declaration specifier missing" "" { target *-*-* } 67 } */

  int (^c)(char);
  (1 ? c : 0)('x');
  (1 ? 0 : c)('x');

  (1 ? c : c)('x');
}
