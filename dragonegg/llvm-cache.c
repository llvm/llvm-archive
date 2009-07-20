/* Caching values "in" trees
Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
Contributed by Chris Lattner (sabre@nondot.org)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

//===----------------------------------------------------------------------===//
// This code lets you to associate a void* with a tree, as if it were cached
// inside the tree: if the tree is garbage collected and reallocated, then the
// cached value will have been cleared.
//===----------------------------------------------------------------------===//

// Plugin headers
#include "llvm-cache.h"

// GCC headers
#include "hashtab.h"

#include "stdio.h" //QQ

#define tree_llvm_map_eq tree_map_base_eq
#define tree_llvm_map_hash tree_map_base_hash
#define tree_llvm_map_marked_p tree_map_base_marked_p

static int debugging_tree_llvm_map_marked_p (const void *p) {//QQ
  printf("debugging_ggc call for %p\n", p);
  return tree_map_base_marked_p(p);
}

static GTY ((if_marked ("debugging_tree_llvm_map_marked_p"),
             param_is (struct tree_llvm_map)))
  htab_t llvm_cache;

bool llvm_has_cached (union tree_node *t) {
  struct tree_map_base in;

  if (!llvm_cache)
    return false;

  in.from = t;
  return htab_find (llvm_cache, &in) != NULL;
}

const void *llvm_get_cached (union tree_node *t) {
  struct tree_llvm_map *h;
  struct tree_map_base in;

  if (!llvm_cache)
    return NULL;

  in.from = t;
  h = (struct tree_llvm_map *) htab_find (llvm_cache, &in);
  return h ? h->val : NULL;
}

const void *llvm_set_cached (union tree_node *t, const void *val) {
  struct tree_llvm_map **slot;
  struct tree_map_base in;

  if (!llvm_cache)
    llvm_cache = htab_create_ggc (1024, tree_llvm_map_hash, tree_llvm_map_eq, NULL);

  in.from = t;

  slot = (struct tree_llvm_map **) htab_find_slot (llvm_cache, &in, INSERT);
  gcc_assert(slot);

  if (!*slot) {
    *slot = GGC_NEW (struct tree_llvm_map);
    (*slot)->base.from = t;
  }

  (*slot)->val = val;
}
