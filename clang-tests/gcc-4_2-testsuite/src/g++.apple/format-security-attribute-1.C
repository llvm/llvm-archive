/* APPLE LOCAL file radar 5096648 */
/* Test for implementation of (__format__ (__CFString__, m, n)) attribute */
/* { dg-options "-Wformat -Wformat-security" } */
/* { dg-do compile { target *-*-darwin* } } */

typedef const struct __CFString * CFStringRef;
#define CFSTR(cStr)  ((CFStringRef) __builtin___CFStringMakeConstantString ("" cStr ""))  /* { dg-warning "expanded from macro" } */
#define SECURITY_ATTR	__attribute__ ((__format__ (__CFString__, 1, 2)))
extern void CFLog(CFStringRef format, ...) SECURITY_ATTR;

int d;
const char *string;
int main()
{
	CFStringRef foo;
        CFLog (foo);	/* { dg-warning "format string is not a string literal" } */	
	CFLog (foo, d);		// ok
	CFLog(CFSTR("foo is %@"), CFSTR("foo is %@"), foo);	/* { dg-warning "data argument not used by format string" } */
	CFLog(CFSTR("foo is %@"), CFSTR("foo is %@"));	// OK
	CFLog(CFSTR("foo is %@"), foo);	// OK
	CFLog(CFSTR("foo is %@"));			/* { dg-warning "more '%' conversions than data arguments" } */
}
