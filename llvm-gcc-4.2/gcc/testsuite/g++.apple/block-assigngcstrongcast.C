/* APPLE LOCAL file radar 5832193 */
/* assigning a Block into an struct slot should elicit a write-barrier under GC */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.5 -ObjC++ -fobjc-gc -framework Foundation" { target *-*-darwin* } } */

#ifdef objc_assign_ivar
#define __objc_assign_ivar objc_assign_ivar
#endif
#include <Foundation/Foundation.h>

#undef objc_assign_ivar

void * _NSConcreteStackBlock;
void _Block_byref_assign_copy(void * dst, void *src){}
void _Block_byref_release(void*src){}

int GlobalInt = 0;

id objc_assign_global(id val, id *dest) {
    GlobalInt = 1;
    return (id)0;
}

id objc_assign_ivar(id val, id dest, ptrdiff_t offset) {
    GlobalInt = 1;
    return (id)0;
}

id objc_assign_strongCast(id val, id *dest) {
    GlobalInt = 1;
    return (id)0;
}

typedef struct {
    void (^ivarBlock)(void);
} StructWithBlock_t;


int main(char *argc, char *argv[]) {
   StructWithBlock_t *swbp = (StructWithBlock_t *)malloc(sizeof(StructWithBlock_t*));
   __byref int i = 10;
   // assigning a Block into an struct slot should elicit a write-barrier under GC
   swbp->ivarBlock = ^ { ++i; };
   return GlobalInt - 1;
}
