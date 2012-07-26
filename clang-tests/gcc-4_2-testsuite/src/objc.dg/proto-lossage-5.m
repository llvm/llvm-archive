/* Do not lose references to forward-declared protocols.  */
/* { dg-do compile } */
@class MyBaseClass;
@class MyClassThatFails;
@protocol _MyProtocol;

__attribute__((objc_root_class)) @interface MyClassThatFails
- (MyBaseClass<_MyProtocol> *) aMethod;
@end

__attribute__((objc_root_class)) @interface MyBaseClass
@end

@protocol _MyProtocol
@end

@implementation MyClassThatFails
- (MyBaseClass<_MyProtocol> *) aMethod
{
    return 0;
}
@end
