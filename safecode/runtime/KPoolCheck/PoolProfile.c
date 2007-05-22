/* Profiling support:

   Each Metapool contains a pointer to a profile tree.  this tree
   tracks profile call site and frequency A global tree is also
   maintained that tracks all metapools.
*/

#include "PoolCheck.h"
#include "adl_splay.h"

extern int printk(const char *fmt, ...);

static void* allmp = 0;
static int profile_pause = 0;

void pchk_profile(MetaPoolTy* MP, void* pc) {
  if (profile_pause) return;
  if (!MP) return;

  adl_splay_insert(&allmp, MP, 0, 0);
  if (!adl_splay_find(&MP->profile, pc))
    adl_splay_insert(&MP->profile, pc, 1, 0);

  void* key = pc;
  unsigned tag;
  
  adl_splay_retrieve(&MP->profile, &key, 0, (void**)&tag);
  adl_splay_insert(&MP->profile, key, 1, (void*)tag);
}

void print_item(void* p, unsigned l, void* t) {
  printk("(0x%x, %d) ", p, (unsigned) t);
}

void print_pool(void* p, unsigned l, void* t) {
  printk("[0x%x ", p);
  adl_splay_foreach(&(((MetaPoolTy*)p)->profile), print_item);
  printk("]\n");
}

void pchk_profile_print() {
  profile_pause = 1;
  
  adl_splay_foreach(&allmp, print_pool);
  
  profile_pause = 0;  
}

