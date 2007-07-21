/* APPLE LOCAL file 4436477 */
extern void abort();
/* { dg-do run { target powerpc*-*-darwin* } } */
/* { dg-options "-std=gnu99" } */

#pragma reverse_bitfields on

typedef struct _S5
{
 unsigned long l1 : 16;
 unsigned long l2 : 16;
 unsigned char c1;
 unsigned short s1 : 8;
 unsigned short s2 : 8;
 unsigned char c2;
} S5;

union U { S5 ss; unsigned int x[4]; };

int TestS5(void)
{
 union U u = {0};
 
 u.ss.c1 = 0x56;
 u.ss.c2 = 0x78;
 
 if (sizeof(S5) != 8 ||
     u.ss.s1 != 0x56 || u.ss.s2 != 0x78 ||
     u.x[0] != 0x0000000 || u.x[1] != 0x56000078 || u.x[2] != 0x00000000)
    abort();
 return 0;
}

int main()
{
 return TestS5();
}
