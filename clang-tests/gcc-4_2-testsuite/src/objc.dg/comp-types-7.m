/* { dg-do compile } */
/* We used to ICE because we removed the cast to List_linked*
   in -[ListIndex_linked next]. */

__attribute__((objc_root_class)) @interface List
{
@public
  int firstLink;
}
@end

__attribute__((objc_root_class)) @interface ListIndex_linked
{
@public
  List *collection;
  int link;
}
@end

@interface List_linked: List
@end

@implementation List
@end

@implementation ListIndex_linked
- next
{
   link = ((List_linked*)collection)->firstLink;
}
@end

