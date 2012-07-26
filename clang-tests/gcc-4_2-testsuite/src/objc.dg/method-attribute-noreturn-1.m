/* APPLE LOCAL file radar 4727659 */
/* Check for use of attribute 'noreturn' on methods. */

extern void exit (int);

__attribute__((objc_root_class)) @interface INTF
- (void) noret __attribute__ ((noreturn));
- (void) noretok __attribute__ ((noreturn));
+ (void) c_noret __attribute__ ((noreturn));
+ (void) c_noretok __attribute__ ((noreturn));
@end

@implementation INTF
- (void) noret {}   /* { dg-warning "function declared \'noreturn\' should not return" } */
+ (void) c_noret {} /* { dg-warning "function declared \'noreturn\' should not return" } */
+ (void) c_noretok { exit(0); } // ok
- (void) noretok { exit (0); } // ok
@end
