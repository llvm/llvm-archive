/* APPLE LOCAL file 4531482 */
/* Test that __OBJC2,__nonlazy_class and __OBJC2,__nonlazy_catgry sections are
   emitted with -fobjc-abi-version=3. */
/* { dg-options "-fobjc-abi-version=3" } */
/* { dg-do compile } */

// test.m
@interface Test @end
@implementation Test +load { } @end
@implementation Test (Category) +load { } @end
/* { dg-final { scan-assembler "LABEL_NONLAZY_CLASS" } } */
/* { dg-final { scan-assembler "LABEL_NONLAZY_CATEGORY" } } */
