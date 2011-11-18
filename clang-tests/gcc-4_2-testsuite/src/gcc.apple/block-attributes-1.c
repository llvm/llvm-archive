/* APPLE LOCAL file radar 6237713 - modified for radar 6230297 */
/* Test for using attributes on blocks. */
/* { dg-options "-mmacosx-version-min=10.5 -fblocks" { target *-*-darwin* } } */
/* { dg-do compile } */

extern void exit(int);

int (^a)(int) __attribute__((noreturn));

int main()
{
       void (^a)(void) __attribute__((noreturn)) = ^ (void) __attribute__((noreturn)) { exit (0); }; // OK;

	int (^b)(int) __attribute__((noreturn)) = ^ (int i) __attribute__((noreturn)) { return i; }; /* { dg-warning "block declared \\'noreturn\\' should not return" } */

	int (^c)(void) __attribute__((noreturn)) = ^ __attribute__((noreturn)) { return 100; }; /* { dg-warning "block declared \\'noreturn\\' should not return" } */

	a = ^ (void) __attribute__((noreturn)) { exit (0); }; // OK

        a = ^ (void) __attribute__((noreturn)) { ; };	/* { dg-warning "\\'noreturn\\' function does return" "" { xfail *-*-* } } <rdar://problem/10466107> */

}

