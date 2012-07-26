/* Check that the compiler does correctly complain about
   exceptions being caught by previous @catch blocks.  */
/* Force the use of NeXT runtime to see that we don't ICE after
   generating the warning message.  */

/* { dg-do compile } */
/* { dg-options "-Wall -fnext-runtime -fobjc-exceptions" } */

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
    @catch (Exception* e) {	/* { dg-warning "earlier handler" } */
    }
    @catch (FooException* fe) {	/* { dg-warning "will be caught" } */
    }
}

