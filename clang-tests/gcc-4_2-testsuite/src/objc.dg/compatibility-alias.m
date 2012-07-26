/* APPLE LOCAL file radar 5835805 */
/* Test use of alias class name in defining a new class and its super class
   results in proper diagnostics if class/super class is being mis-used.
*/
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface Super @end

__attribute__((objc_root_class)) @interface MyWpModule @end 	/* { dg-error "previous definition is here" } */

@compatibility_alias  MyAlias MyWpModule;

@compatibility_alias  AliasForSuper Super;

@interface MyAlias : AliasForSuper /* { dg-error "duplicate interface definition for class 'MyWpModule'" } */
@end

@implementation MyAlias : AliasForSuper
@end

