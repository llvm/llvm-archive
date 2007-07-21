/* APPLE LOCAL file radar 4805321 */
/* Test for property lookup in a protocol id. */
/* { dg-options "-fobjc-new-property" } */
/* { dg-do compile { target *-*-darwin* } } */

@protocol NSCollection
@property(assign, readonly) int  count;
@end

static int testCollection(id <NSCollection> collection) {
    return collection.count;
}
