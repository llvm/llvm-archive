/* APPLE LOCAL file 4649718, 4651088 */
/* Test for bogus property declarations. */
/* { dg-do compile } */

@interface MyClass {

};
@property (ivar) unsigned char bufferedUTF8Bytes[4]; /* { dg-error "bad property declaration" } */
@property (ivar) unsigned char bufferedUTFBytes:1;   /* { dg-error "bad property declaration" } */
@end

@implementation MyClass
@end

