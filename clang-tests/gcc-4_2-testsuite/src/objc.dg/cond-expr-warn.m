/* APPLE LOCAL file radar 6231433 */
/* { dg-do compile } */

__attribute__((objc_root_class)) @interface NSKey @end
__attribute__((objc_root_class)) @interface UpdatesList @end

void foo (int i, NSKey *NSKeyValueCoding_NullValue, UpdatesList *nukedUpdatesList)
{
  i ? NSKeyValueCoding_NullValue : nukedUpdatesList; /* { dg-warning "conditional expression" } */
}
