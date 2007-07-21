/* APPLE LOCAL file 4430139 */

/* CW layout of this one is truly bizarre:
   0000000000aaaaaabbbbbbbbbb000000
   0000000000cccccccccccccccccccccc
*/

extern void abort();
/* { dg-do run { target powerpc*-*-darwin* } } */
/* { dg-options "-std=gnu99" } */
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma reverse_bitfields on

#define USE_STRUCT_WRAPPER 0

#pragma pack(push, 2)
typedef struct 
{
   short a : 6;
   int b : 10;
   int c : 22;
} Foo;
typedef union { Foo x; int y[6]; } u1;
#pragma pack(pop)


int main(int argc, char* argv[])
{
    int i;
    u1 U;
    for (i=0; i<6; i++)
	U.y[i] = 0;
    U.x.a = 1;
    U.x.b = 3;
    U.x.c = 5;

    if (sizeof(Foo) != 8 
        || U.y[0] != 0x000100c0
	|| U.y[1] != 0x00000005)
    abort();
   return 0;
}
