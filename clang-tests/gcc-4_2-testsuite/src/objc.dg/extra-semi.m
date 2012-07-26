/* Allow extra semicolons in between method declarations,
   for old times' sake.  */

/* { dg-do compile } */

__attribute__((objc_root_class)) @interface Foo
   -(Foo *) expiration;
   -(void) setExpiration:(Foo *) date;;
   -(int) getVersion;
@end
