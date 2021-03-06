/* APPLE LOCAL file constant CFStrings */
/* Test whether the __builtin__CFStringMakeConstantString 
   "function" fails gracefully when handed a non-constant
   argument.  This will only work on MacOS X 10.1.2 and later.  */
/* Developed by Ziemowit Laski <zlaski@apple.com>.  */

/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-fconstant-cfstrings" } */

typedef const struct __CFString *CFStringRef;

#ifdef __CONSTANT_CFSTRINGS__
#define CFSTR(STR)  ((CFStringRef) __builtin___CFStringMakeConstantString (STR)) /* { dg-warning "expanded from macro 'CFSTR'" } */
#else
#error __CONSTANT_CFSTRINGS__ not defined
#endif

extern int cond;
extern const char *func(void);

const CFStringRef s0 = CFSTR("Hello" "there");

int main(void) {
  CFStringRef s1 = CFSTR("Str1");
  CFStringRef s2 = CFSTR(cond? "Str2": "Str3"); /* { dg-error "CFString literal is not a string" } */
  CFStringRef s3 = CFSTR(func());  /* { dg-error "CFString literal is not a string" } */

  return 0;
}
