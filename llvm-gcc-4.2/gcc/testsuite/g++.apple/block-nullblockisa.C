/* APPLE LOCAL file radar 6244520 */
/* { dg-do run } */
/* { dg-options "-mmacosx-version-min=10.6 -fblocks" { target *-*-darwin* } } */

#include <stdio.h>
#include <stdlib.h>

#define HASISA 1

void check(void *ptr) {
    struct inner {
#if HASISA
        void* isa; // should be zero
#endif
        long forwarding;
        int flags;
        int size;
        // long copyhelper not needed
        // long disposehelper not needed
        int i;  //
    };
    struct block_with_blocki {
        long isa;
        int flags;
        int size;
        long impl;
        long copyhelper;
        long destroyhelper;
        struct inner *blocki;
    } *block = (struct block_with_blocki *)ptr;
    // sanity checks
    if (block->size != sizeof(struct block_with_blocki)) {
        // layout funny
        printf("layout is funny, struct size is %d vs runtime size %ld\n",
                    block->size,
                    sizeof(struct block_with_blocki));
        exit(1);
    }
    if (block->blocki->size != sizeof(struct inner)) {
        printf("inner is funny; sizeof is %ld, runtime says %d\n", sizeof(struct inner),
            block->blocki->size);
        exit(1);
    }
#if HASISA
    if (block->blocki->isa != (void*)NULL) {
        printf("not a NULL __block isa\n");
        exit(1);
    }
#endif
    return;
}
        
int main(int argc, char *argv[]) {

   __block int i;
   
   check(^{ printf("%d\n", ++i); });
   return 0;
}
   
