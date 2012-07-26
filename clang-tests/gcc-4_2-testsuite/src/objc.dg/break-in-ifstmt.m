/* APPLE LOCAL file radar 4219590 */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface foo
- (void) test;
@end

@implementation foo
-(void) test {
  if (1) {
        break;	/* { dg-error "break" } */
        }
}
@end

