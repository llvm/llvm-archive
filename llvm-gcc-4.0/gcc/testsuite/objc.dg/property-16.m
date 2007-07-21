/* APPLE LOCAL file radar 4660569 */
/* No warning here because accessor methods are INHERITED from NSButton */
#include <AppKit/AppKit.h>

@interface NSButton (Properties)
@property NSString *title;
@end

@implementation NSButton (Properties)
@end
