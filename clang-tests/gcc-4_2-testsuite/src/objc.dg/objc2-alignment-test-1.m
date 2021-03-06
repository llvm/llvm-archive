/* APPLE LOCAL file radar 4561192 */
/* Test that alignment of __class_list sections, among others, are at their 'natural'
   alignment on x86. */
/* { dg-do compile { target i?86-*-darwin* } } */
/* { dg-require-effective-target ilp32 } */
/* { dg-options "-mmacosx-version-min=10.5 -fobjc-abi-version=2" } */

__attribute__((objc_root_class)) @interface c1 @end
__attribute__((objc_root_class)) @interface c2 @end
__attribute__((objc_root_class)) @interface c3 @end
__attribute__((objc_root_class)) @interface c4 @end
__attribute__((objc_root_class)) @interface c5 @end
__attribute__((objc_root_class)) @interface c6 @end
__attribute__((objc_root_class)) @interface c7 @end
__attribute__((objc_root_class)) @interface c8 @end
__attribute__((objc_root_class)) @interface c9 @end

@implementation c1 +load { } +(void)initialize { } @end
@implementation c2 +load { } +(void)initialize { } @end
@implementation c3 +load { } +(void)initialize { } @end
@implementation c4 +load { } +(void)initialize { } @end
@implementation c5 +load { } +(void)initialize { } @end
@implementation c6 +load { } +(void)initialize { } @end
@implementation c7 +load { } +(void)initialize { } @end
@implementation c8 +load { } +(void)initialize { } @end
@implementation c9 +load { } +(void)initialize { } @end

int main() {
    [c1 load];
    [c2 load];
    [c3 load];
    [c4 load];
    [c5 load];
    [c6 load];
    [c7 load];
    [c8 load];
    [c9 load];
}
/* { dg-final { scan-assembler "\t.align 2\nL_OBJC_LABEL_CLASS_\\\$:" } } */
/* { dg-final { scan-assembler "\t.align 2\nL_OBJC_LABEL_NONLAZY_CLASS_\\\$:" } } */
