/* APPLE LOCAL file radar 6116917 */
/* { dg-options "-mmacosx-version-min=10.6 -ObjC -Os -Wall -Wextra" { target *-*-darwin* } } */
/* { dg-do run } */
/* { dg-skip-if "" { powerpc*-*-darwin* } { "-m64" } { "" } } */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <Block_private.h>

struct Block_basic {
    void *isa;
    int Block_flags;  // int32_t
    int Block_size; // XXX should be packed into Block_flags
    void (*Block_invoke)(void *);
    void (*Block_copy)(void *dst, void *src);
    void (*Block_dispose)(void *);
    //long params[0];  // generic space for const & byref hidden params, return value, variable on needs of course
};


void
func(void (^b)(void))
{
	b();
}

void
func2(void (^b)(void))
{
	b();
}

extern char **environ;

int
main(int argc __attribute__((unused)), char *argv[])
{
	struct Block_basic *bb;
	long bbi_addr, bb_addr;

	void (^stage1)(void) = ^{
		void (^stage2)(void) = ^{
			/* trick the compiler into slirping argc/argv into this Block */
			if (environ == argv) {
				fprintf(stdout, "You won the lottery! argv == environ\n");
			}
		};

		func2(stage2);
	};

	bb = (void *)stage1;

	bbi_addr = (long)bb->Block_invoke;
	bb_addr = (long)bb;

	if (labs(bbi_addr - bb_addr) > (64 * 1024)) {
		func(stage1);
		exit(EXIT_SUCCESS);
	} else {
		fprintf(stderr, "Blocks generated code on the stack! Block_copy() is not safe!\n");
		exit(EXIT_FAILURE);
	}
}
