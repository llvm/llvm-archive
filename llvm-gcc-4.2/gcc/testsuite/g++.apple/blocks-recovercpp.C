/* APPLE LOCAL file radar 6214617 */
/* { dg-options "-mmacosx-version-min=10.5 -ObjC++" { target *-*-darwin* } } */
/* { dg-do run } */

#include <stdio.h>
#include <stdlib.h>

int constructors = 0;
int destructors = 0;

void * _NSConcreteStackBlock[32];

#define CONST const

class TestObject
{
public:
	TestObject(CONST TestObject& inObj);
	TestObject();
	~TestObject();
	
	TestObject& operator=(CONST TestObject& inObj);

	int version() CONST { return _version; }
private:
	mutable int _version;
};

TestObject::TestObject(CONST TestObject& inObj)
	
{
        ++constructors;
        _version = inObj._version;
	printf("%p (%d) -- TestObject(const TestObject&) called\n", this, _version); 
}


TestObject::TestObject()
{
        _version = ++constructors;
	printf("%p (%d) -- TestObject() called\n", this, _version); 
}


TestObject::~TestObject()
{
	printf("%p -- ~TestObject() called\n", this);
        ++destructors;
}


TestObject& TestObject::operator=(CONST TestObject& inObj)
{
	printf("%p -- operator= called\n", this);
        _version = inObj._version;
	return *this;
}

void hack(void *block) {
    // check flags to see if constructor/destructor is available;
    struct myblock {
        void *isa;
        int flags;
        int refcount;
        void *invokeptr;
        void (*copyhelper)(struct myblock *dst, struct myblock *src);
        void (*disposehelper)(struct myblock *src);
        long space[32];
    } myversion, *mbp = (struct myblock *)block;
    printf("flags -> %x\n", mbp->flags);
    if (! ((1<<25) & mbp->flags)) {
        printf("no copy/dispose helper functions provided!\n");
        exit(1);
    }
    if (! ((1<<26) & mbp->flags)) {
        printf("no marking for ctor/dtors present!\n");
        exit(1);
    }
    printf("copyhelper -> %p\n", mbp->copyhelper);
    // simulate copy
    mbp->copyhelper(&myversion, mbp);
    if (constructors != 3) {
        printf("copy helper didn't do the constructor part\n");
        exit(1);
    }
    printf("disposehelper -> %p\n", mbp->disposehelper);
    // simulate destroy
    mbp->disposehelper(&myversion);
    if (destructors != 1) {
        printf("dispose helper didn't do the dispose\n");
        exit(1);
    }
}
void testRoutine() {
    TestObject one;
    
    void (^b)(void) = ^{ printf("my copy of one is %d\n", one.version()); };
    hack(b);
}

int main(char *argc, char *argv[]) {
    testRoutine();
    exit(0);
}
