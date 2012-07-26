/* Test -fsyntax-only compiler option  */
/* { dg-do compile } */
/* { dg-options "-fsyntax-only" } */

__attribute__((objc_root_class)) @interface foo
-(void) my_method:(int) i with:(int) j;
@end

@implementation foo
-(void) my_method:(int) i with:(int) j { }
@end
