/* APPLE LOCAL file from 4431497 */
/* Some truly bizarre layouts by CW.  */
/* { dg-do run { target powerpc*-*-darwin* } } */
/* { dg-options "-std=gnu99" } */
extern void abort();
#pragma reverse_bitfields on

#pragma pack(push,1)
typedef struct _S3
{
 unsigned char c1 : 8;
 unsigned short s1 : 8;
 unsigned short s2 : 8;
} S3;

typedef struct _S3W
{
 S3 s3;
 unsigned char c;
}S3W;
/* c1 and s1 overlap.  c and s2 overlap. 
   Size is 3 although 4 bytes are accessed. */
#pragma pack(pop)

typedef union u3 { S3W ss; unsigned int v[6]; } U3;

int TestS3(void)
{
  U3 u = {0};
  u.ss.s3.c1 = 0x56;
  u.ss.s3.s1 = 0x78;
  u.ss.c = 0x90;
  u.ss.s3.s2 = 0xab;
 
  if (sizeof(S3) != 3 || sizeof(S3W) != 4 ||
     u.ss.s3.s1 != 0x78 || u.ss.s3.s2 != 0xab ||
     u.ss.s3.c1 != 0x78 || u.ss.c != 0xab ||
     u.v[0] != 0x780000ab || u.v[1] != 0x00000000)
    abort();
  return 0;
}

#pragma pack(push, 2)
typedef struct _S4
{
 unsigned char c1 : 8;
 unsigned short s1 : 8;
 unsigned short s2 : 8;
} S4;

typedef struct _S4W
{
 S4 s4;
 unsigned char c;
}S4W;
/* c1 and s1 overlap. */
#pragma pack(pop)

typedef union u4 { S4W ss; unsigned int v[6]; } U4;

int TestS4(void)
{
  U4 u = {0};
  u.ss.s4.c1 = 0x56;
  u.ss.s4.s1 = 0x78;
  u.ss.s4.s2 = 0xab;
  u.ss.c = 0x90;
 
  if (sizeof(S4) != 4 || sizeof(S4W) != 6 ||
     u.ss.s4.s1 != 0x78 || u.ss.s4.s2 != 0xab ||
     u.ss.s4.c1 != 0x78 || u.ss.c != 0x90 ||
     u.v[0] != 0x780000ab || u.v[1] != 0x90000000)
    abort();
  return 0;
}

#pragma pack(push,1)
typedef struct _S5
{
 unsigned short s : 16;
 unsigned int l1 : 16;
 unsigned int l2 : 16;
} S5;

typedef struct _S5W
{
 S5 s5;
 unsigned short s;
}S5W;
/* l1 and s5.s overlap.  l2 and s overlap.  Size is 6
   although 8 bytes are referenced. */
#pragma pack(pop)

typedef union u5 { S5W ss; unsigned int v[6]; } U5;

int TestS5(void)
{
 U5 u = {0};
 u.ss.s5.s = 0x5678;
 u.ss.s5.l1 = 0x1289;
 u.ss.s = 0xabcd;
 u.ss.s5.l2 = 0xfe34;
  if (sizeof(S5) != 6 || sizeof(S5W) != 8 ||
     u.ss.s5.l1 != 0x1289 || u.ss.s5.l2 != 0xfe34 ||
     u.ss.s5.s != 0x1289 || u.ss.s != 0xfe34 ||
     u.v[0] != 0x12890000 || u.v[1] != 0x0000fe34 || u.v[2] != 0x00000000)
    abort();
  return 0;
}

#pragma pack(push,2)
typedef struct _S6
{
 unsigned short s : 16;
 unsigned int l1 : 16;
 unsigned int l2 : 16;
} S6;

typedef struct _S6W
{
 S6 s6;
 unsigned short s;
}S6W;
#pragma pack(pop)

typedef union u6 { S6W ss; unsigned int v[6]; } U6;

int TestS6(void)
{
 U6 u = {0};
 u.ss.s6.s = 0x5678;
 u.ss.s6.l1 = 0x9123;
 u.ss.s = 0xabcd;
 u.ss.s6.l2 = 0xef04;
  if (sizeof(S6) != 8 || sizeof(S6W) != 10 ||
     u.ss.s6.l1 != 0x9123 || u.ss.s6.l2 != 0xef04 ||
     u.ss.s6.s != 0x9123 || u.ss.s != 0xabcd ||
     u.v[0] != 0x91230000 || u.v[1] != 0x0000ef04 || u.v[2] != 0xabcd0000)
    abort();
 return 0;
}
int main()
{
 return TestS3() + TestS4() + TestS5() + TestS6();
}
