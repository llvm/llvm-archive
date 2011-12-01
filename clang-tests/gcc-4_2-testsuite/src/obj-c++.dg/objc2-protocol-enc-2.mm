/* APPLE LOCAL file radar 10492418 */
/* Check for extended method type encodings in protocols: class names and 
   block object parameter types */
/* { dg-options "-mmacosx-version-min=10.5 -fobjc-abi-version=2" { target powerpc*-*-darwin* i?86*-*-darwin* } } */
/* { dg-do compile { target *-*-darwin* } } */

@class NSObject;
@protocol NSCopying @end

@protocol Proto
-(id)m1:(id<NSCopying>)a;
+(NSObject*)m2:(NSObject<NSCopying>*)a;
@optional
-(id(^)(id))m3:(id<NSCopying>(^)(id<NSCopying>))a;
+(NSObject*(^)(NSObject*))m4:(NSObject<NSCopying>*(^)(NSObject<NSCopying>*))a;
@end

int main()
{
    (void)@protocol(Proto);
    return 0;
}

/* m1: .asciz "@24@0:8@\"<NSCopying>\"16" */
/* { dg-final { scan-assembler "\"@\[0-9\]*@\[0-9\]*:\[0-9\]*@\\\\\"<NSCopying>\\\\\"\[0-9\]*\"" } } */

/* m2: .asciz "@\"NSObject\"24@0:8@\"NSObject<NSCopying>\"16" */
/* { dg-final { scan-assembler "\"@\\\\\"NSObject\\\\\"\[0-9\]*@\[0-9\]*:\[0-9\]*@\\\\\"NSObject<NSCopying>\\\\\"\[0-9\]*\"" } } */

/* m3: .asciz "@?<@@?@>24@0:8@?<@\"<NSCopying>\"@?@\"<NSCopying>\">16" */
/* { dg-final { scan-assembler "\"@\\\?<@@\\\?@>\[0-9\]*@\[0-9\]*:\[0-9\]*@\\\?<@\\\\\"<NSCopying>\\\\\"@\\\?@\\\\\"<NSCopying>\\\\\">\[0-9\]*\"" } } */

/* m4: .asciz "@?<@\"NSObject\"@?@\"NSObject\">24@0:8@?<@\"NSObject<NSCopying>\"@?@\"NSObject<NSCopying>\">16" */
/* { dg-final { scan-assembler "\"@\\\?<@\\\\\"NSObject\\\\\"@\\\?@\\\\\"NSObject\\\\\">\[0-9\]*@\[0-9\]*:\[0-9\]*@\\\?<@\\\\\"NSObject<NSCopying>\\\\\"@\\\?@\\\\\"NSObject<NSCopying>\\\\\">\[0-9\]*\"" } } */
