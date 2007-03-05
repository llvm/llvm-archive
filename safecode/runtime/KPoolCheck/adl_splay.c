#include "adl_splay.h"

typedef struct tree_node Tree;
struct tree_node {
  Tree* left;
  Tree* right;
  char* key;
  unsigned len;
  void* tag;
};

/* Memory management functions */

static Tree* freelist = 0;
static void* (*ext_alloc)(unsigned) = 0;


static inline Tree* tmalloc() {
  if(freelist) {
    Tree* t = freelist;
    freelist = freelist->left;
    return t;
  } else
    return (Tree*) ext_alloc(sizeof(Tree));
}

static inline void tfree(Tree* t) {
  t->left = freelist;
  freelist = t;
}


static int key_lt(char* key, Tree* t) {
  return (key < t->key);
}

static int key_gt(char* key, Tree* t) {
  return (key >= t->key + t->len);
}

static inline Tree* rotate_right(Tree* p) {
  Tree* x = p->left;
  p->left = x->right;
  x->right = p;
  return x;
}

static inline Tree* rotate_left(Tree* p) {
  Tree* x = p->right;
  p->right = x->left;
  x->left = p;
  return x;
}

/* This function by D. Sleator <sleator@cs.cmu.edu> */
static Tree* splay (Tree * t, char* key) {
  Tree N, *l, *r, *y;
  if (t == 0) return t;
  N.left = N.right = 0;
  l = r = &N;

  while(1) {
    if (key_lt(key, t)) {
      if (t->left == 0) break;
      if (key_lt(key, t->left)) {
        y = t->left;                           /* rotate right */
        t->left = y->right;
        y->right = t;
        t = y;
        if (t->left == 0) break;
      }
      r->left = t;                               /* link right */
      r = t;
      t = t->left;
    } else if (key_gt(key, t)) {
      if (t->right == 0) break;
      if (key_gt(key, t->right)) {
        y = t->right;                          /* rotate left */
        t->right = y->left;
        y->left = t;
        t = y;
        if (t->right == 0) break;
      }
      l->right = t;                              /* link left */
      l = t;
      t = t->right;
    } else {
      break;
    }
  }
  l->right = t->left;                                /* assemble */
  r->left = t->right;
  t->left = N.right;
  t->right = N.left;
  return t;
}

/* My version, needs testing */
static Tree* my_splay(Tree* t, char* key) {
  if (!t) return t;
  
  while (1) {
    if (key_lt(key, t)) {
      if (!t->left) return t;
      else if (key_lt(key, t->left))
        t = rotate_right(rotate_right(t));
      else if (key_gt(key, t->left)) {
        t->left = rotate_left(t->left);
        t = rotate_right(t);
      } else
        return  rotate_right(t);
    } else if (key_gt(key, t)) {
      if (!t->right) return t;
      else if (key_gt(key, t->right))
        t = rotate_left(rotate_left(t));
      else if (key_lt(key, t->right)) {
        t->right = rotate_right(t->right);
        t = rotate_left(t);
      } else
        return rotate_left(t);
    } else
      return t;
  }
}

static inline Tree* insert(Tree* t, char* key, unsigned len, void* tag) {
  Tree* n = 0;
  t = splay(t, key);
  if (t && !key_lt(key, t) && !key_gt(key, t)) return t; /* already in */
  n = tmalloc();
  n->key = key;
  n->len = len;
  n->tag = tag;
  n->right = n->left = 0;
  if (t) {
    if (key_lt(key, t)) {
      n->left = t->left;
      n->right = t;
      t->left = 0;
    } else {
      n->right = t->right;
      n->left = t;
      t->right = 0;
    }
  }
  return n;
}

static inline Tree* delete(Tree* t, char* key) {
  if (!t) return t;
  t = splay(t, key);
  if (!key_lt(key, t) && !key_gt(key, t)) {
    Tree* x = 0;
    if (!t->left)
      x = t->right;
    else {
      x = splay(t->left, key);
      x->right = t->right;
    }
    tfree(t);
    return x;
  }
  return t; /* not there */
}

static int count(Tree* t) {
  if (t)
    return 1 + count(t->left) + count(t->right);
  return 0;
}

/* return any node with the matching tag */
static Tree* find_tag(Tree* t, void* tag) {
  if (t) {
    Tree* n = 0;
    if (t->tag == tag) return t;
    if ((n = find_tag(t->left, tag))) return n;
    if ((n = find_tag(t->right, tag))) return n;
    return 0;
  }
}

/* interface */

void adl_splay_insert(void** tree, void* key, unsigned len, void* tag)
{
  *(Tree**)tree = insert(*(Tree**)tree, (char*)key, len, tag);
}

void adl_splay_delete(void** tree, void* key)
{
  *(Tree**)tree = delete(*(Tree**)tree, (char*)key);
}

void adl_splay_delete_tag(void** tree, void* tag) {
  Tree* t = *(Tree**)tree;
  Tree* n = find_tag(t, tag);
  while (n) {
    t = delete(t, n->key);
    n = find_tag(t, tag);
  }
  *(Tree**)tree = t;
}

int  adl_splay_find(void** tree, void* key) {
  Tree* t = splay(*(Tree**)tree, (char*)key);
  *(Tree**)tree = t;
  return (t && !key_lt((char*)key, t) && !key_gt((char*)key, t));
}

int adl_splay_retrieve(void** tree, void** key, unsigned* len, void** tag) {
  Tree* t = splay(*(Tree**)tree, (char*)key);
  *(Tree**)tree = t;
  if (t && !key_lt((char*)key, t) && !key_gt((char*)key, t)) {
    *key = t->key;
    if (len) *len = t->len;
    if (tag) *tag = t->tag;
    return 1;
  }
  return 0;
}

int  adl_splay_size(void** tree) {
  return (count(*(Tree**)tree));
}

void* adl_splay_any(void** tree) {
  if (*(Tree**)tree)
    return ((*(Tree**)tree)->key);
  return 0;
}

void adl_splay_libinit(void* (nodealloc)(unsigned) ) {
  ext_alloc = nodealloc;
}

void adl_splay_libfini(void (nodefree)(void*) ) {
  while (freelist) {
    Tree* n = freelist->left;
    nodefree(freelist);
    freelist = n;
  }
}


#if 0
#include <stdio.h>
#include <stdlib.h>

int main() {
  adl_splay_libinit((void* (*)(int))malloc);
  void* t = 0;
  long x;
  for (x = 0; x < 100; ++x) {
    adl_splay_insert(&t, (void*)x, 10, 0);
  }

  printf("Size after 100 inserts of size 10 (overlap): %d\n", adl_splay_size(&t));

  for (x = 0; x < 100; ++x) {
    int f = adl_splay_find(&t, (void*)x);
    if (!f) printf("Failed find!\n");
  }
  for (x = 0; x < 100; x += 20) {
    int f = adl_splay_find(&t, (void*)x);
    if (!f) printf("Failed find!\n");
  }

  for (x = 0; x < 100; ++x) {
    adl_splay_delete(&t, (void*)x);
  }

  printf("Size should be 0: %d\n", adl_splay_size(&t));
  return 0;
}

#endif
