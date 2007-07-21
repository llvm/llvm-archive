/* APPLE LOCAL file radar 4506903 */
/* Test for property lookup in a protocol id. */
/* { dg-do compile { target *-*-darwin* } } */

@protocol NSCollection
@property(readonly) int  count;
@end

static int testCollection(id <NSCollection> collection) {
    return collection.count;
}
