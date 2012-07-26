/* Test if compiler detects object as an parameter to a method
   or not. It is not valid.  */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface foo
@end

@implementation foo
@end

__attribute__((objc_root_class)) @interface bar
-(void) my_method:(foo) my_param; /* { dg-error "can not use an object as parameter to a method" } */
@end

@implementation bar
-(void) my_method:(foo) my_param /* { dg-error "can not use an object as parameter to a method" } */
{
}
@end

