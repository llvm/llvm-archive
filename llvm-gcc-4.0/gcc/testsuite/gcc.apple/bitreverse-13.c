/* APPLE LOCAL file 4350099 */
/* { dg-do run { target powerpc*-*-darwin* } } */
extern void abort ();
extern int printf(const char *, ...);
typedef struct _foo1 FOO1;

struct _foo1
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

FOO1 vDelim1 = {1, 0, 0, 0, 0, 0, 0, 0};
void f1() {
  union {
   unsigned int x;
   FOO1 y;
  } u;
  u.y = vDelim1;
  if (u.x != 0x80000000U)
    abort ();
}
/***********************************************/
#pragma reverse_bitfields on

typedef struct _foo2 FOO2;

struct _foo2
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

FOO2 vDelim2 = {1, 0, 0, 0, 0, 0, 0, 0};
void f2() {
  union {
   unsigned int x;
   FOO2 y;
  } u;
  u.y = vDelim2;
  if (u.x != 1)
    abort();
}
#pragma reverse_bitfields off
/****************************************************/
#pragma reverse_bitfields on

typedef struct _foo3 FOO3;

#pragma reverse_bitfields off
struct _foo3
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

FOO3 vDelim3 = {1, 0, 0, 0, 0, 0, 0, 0};
void f3() {
  union {
   unsigned int x;
   FOO3 y;
  } u;
  u.y = vDelim3;
  if (u.x != 0x80000000U)
    abort ();
}
#pragma reverse_bitfields off
/****************************************************/
#pragma reverse_bitfields on

struct _foo4
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

typedef struct _foo4 FOO4;

FOO4 vDelim4 = {1, 0, 0, 0, 0, 0, 0, 0};
void f4() {
  union {
   unsigned int x;
   FOO4 y;
  } u;
  u.y = vDelim4;
  if (u.x != 1)
    abort();
}
#pragma reverse_bitfields off
/****************************************************/
#pragma reverse_bitfields on

struct _foo5
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

#pragma reverse_bitfields off
typedef struct _foo5 FOO5;

FOO5 vDelim5 = {1, 0, 0, 0, 0, 0, 0, 0};
void f5() {
  union {
   unsigned int x;
   FOO5 y;
  } u;
  u.y = vDelim5;
  if (u.x != 1)
    abort();
}
#pragma reverse_bitfields off
/****************************************************/
#pragma reverse_bitfields off

typedef struct _foo6 FOO6;

#pragma reverse_bitfields on
struct _foo6
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

FOO6 vDelim6 = {1, 0, 0, 0, 0, 0, 0, 0};
void f6() {
  union {
   unsigned int x;
   FOO6 y;
  } u;
  u.y = vDelim6;
  if (u.x != 1)
    abort();
}
#pragma reverse_bitfields off
/****************************************************/
#pragma reverse_bitfields off

struct _foo7
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

typedef struct _foo7 FOO7;

FOO7 vDelim7 = {1, 0, 0, 0, 0, 0, 0, 0};
void f7() {
  union {
   unsigned int x;
   FOO7 y;
  } u;
  u.y = vDelim7;
  if (u.x != 0x80000000U)
    abort ();
}
#pragma reverse_bitfields off
/****************************************************/
#pragma reverse_bitfields off

struct _foo8
{
        unsigned int fTab:1,
                fSpace:1,
                fComma:1,
                fSemiColon:1,
                fCustom:1,
                fConsecutive:1, 
                iTextDelm:2,
                chCustom:16; 
};

#pragma reverse_bitfields on
typedef struct _foo8 FOO8;

FOO8 vDelim8 = {1, 0, 0, 0, 0, 0, 0, 0};
void f8() {
  union {
   unsigned int x;
   FOO8 y;
  } u;
  u.y = vDelim8;
  if (u.x != 0x80000000U)
    abort ();
}
#pragma reverse_bitfields off
/****************************************************/
main() {
  f1(); f2(); f3(); f4(); f5(); f6(); f7(); f8();
  return 0;
}
