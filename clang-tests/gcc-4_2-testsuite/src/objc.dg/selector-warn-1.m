/* APPLE LOCAL file radar 4441551 */
/* Test warning for typecast of a selector. */
/* { dg-options "-Wobjc2 " } */
/* { dg-do compile } */

typedef struct objc_object { struct objc_class *class_pointer; } *id;
typedef struct objc_selector    *SEL;

__attribute__((objc_root_class)) @interface Foo
- (char*) foo;
- (void) bar;
@end

@implementation Foo
- (void) bar
{
}

- (char*) foo
{
  char* a,b,c;
  a = (char*)@selector(bar); /* { dg-warning "type-cast of @selector expression" } */
  return (char*)@selector(bar); /* { dg-warning "type-cast of @selector expression" } */
}
@end

