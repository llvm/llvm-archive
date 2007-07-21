/* APPLE LOCAL file radar 4805321 */
/* Test for correct name lookup in presence of multiple protocols. */
/* { dg-options "-fobjc-new-property -std=c99" } */
/* { dg-do compile { target *-*-darwin* } } */

@protocol NSCoding
@end

@protocol LKObject <NSCoding>
@end

@protocol LKTiming
@property (assign) double duration;
@end

@interface LKAnimation <LKObject, LKTiming>
@end

static void test (void)
{
    LKAnimation *anim;
    anim.duration = 1.0;
}

