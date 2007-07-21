/* APPLE LOCAL file 4430139 */
extern void abort();
#include <string.h>
/* This is bitreverse-18 with d and e switched.
   Note that this changes the layout and even the size. */

/* { dg-do run { target powerpc*-*-darwin* } } */
/* { dg-options "-std=gnu99" } */

typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma reverse_bitfields on

#define USE_STRUCT_WRAPPER 0

#pragma pack(push, 2)
typedef struct 
{
	DWORD	a;
#if USE_STRUCT_WRAPPER
	struct {
#endif
	WORD	b:2,
		c:14;
#if USE_STRUCT_WRAPPER
	};
#endif
	DWORD	d:10,
		e:22;
	DWORD	f;
	DWORD	g:25,
		h:1,
		i:6;
#if USE_STRUCT_WRAPPER
	struct {
#endif
	WORD	j:14,
		k:2;
#if USE_STRUCT_WRAPPER
	};
#endif
} Foo;
typedef union { Foo x; int y[6]; } u1;
#pragma pack(pop)


int main(int argc, char* argv[])
{
    Foo foo;
    int i;
    u1 U;
    memset (&U, 0, sizeof(u1));
    U.x.a = 1;
    U.x.b = 1;
    U.x.c = 1;
    U.x.d = 1;
    U.x.e = 1;
    U.x.f = 1;
    U.x.g = 1;
    U.x.h = 1;
    U.x.i = 1;
    U.x.j = 1;
    U.x.k = 1;

    int s = sizeof(Foo);

    if (sizeof(Foo) != 22
	|| U.y[0] != 0x00000001
	|| U.y[1] != 0x00010000
	|| U.y[2] != 0x00000001
	|| U.y[3] != 0x00000001
	|| U.y[4] != 0x06000001
	|| U.y[5] != 0x40010000)
      abort();
    return 0;
}
