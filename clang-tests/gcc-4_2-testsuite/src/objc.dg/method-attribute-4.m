/* APPLE LOCAL file radar 6049617 */
/* Test of implementation of method attributes on class methods. */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface NSString 
- (const char *)lossyCString __attribute__((deprecated)); /* { dg-warning "declared here" } */
+ (const char *)stringWithContentsOfFile __attribute__((deprecated)); /* { dg-warning "declared here" } */
@end

void foo (NSString *p)
{
    id pid;
        Class pclass;

    [p lossyCString];    /* { dg-warning "\\'lossyCString\\' is deprecated" } */
    [NSString stringWithContentsOfFile]; /* { dg-warning "\\'stringWithContentsOfFile\\' is deprecated" } */

    [pid lossyCString];
    [pclass stringWithContentsOfFile];      // OK
}

