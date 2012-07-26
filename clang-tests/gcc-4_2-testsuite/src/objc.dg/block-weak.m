/* APPLE LOCAL file __block copy/dispose 7284529 */
/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-fblocks" } */

__attribute__((objc_root_class)) @interface ShortcutsController
@end

@class barbar;
@implementation ShortcutsController
- (void) _markConflicts
{
 __attribute__((__blocks__(byref))) barbar * matchDict;
 __attribute__((__blocks__(byref))) void (^markConflictsBlock)();
}
@end
