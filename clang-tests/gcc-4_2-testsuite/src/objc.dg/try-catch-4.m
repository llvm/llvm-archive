/* Check that the compiler does not incorrectly complain about
   exceptions being caught by previous @catch blocks.  */
/* Author: Ziemowit Laski <zlaski@apple.com> */

/* { dg-do compile } */
/* { dg-options "-Wall -fobjc-exceptions" } */

__attribute__((objc_root_class)) @interface Exception
@end

@interface FooException : Exception
@end

extern void foo();

void test()
{
    @try {
        foo();
    }
    @catch (FooException* fe) {
    }
    @catch (Exception* e) {
    }
}

