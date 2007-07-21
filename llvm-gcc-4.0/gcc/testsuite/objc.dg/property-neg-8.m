/* APPLE LOCAL file 4656712 */
/* Issue error if user declares his own setter/getter while specifying a 
   property which must be synthesized and needs to synthesize its own 
   setter/getter. */
/* { dg-do compile } */

typedef const char SCMLog;

@interface INTF
@property (ivar) SCMLog *log;
- (SCMLog *) log;
- (void) setLog:(SCMLog *)log;
@end		/* { dg-error "user accessor '-log' not allowed" } */
		/* { dg-error "user accessor '-setLog:' not allowed" "" { target *-*-* } 13 } */
@implementation INTF @end
