// Place to keep various things that will need to be sorted out someday.
#ifndef BITS_AND_BOBS_H
#define BITS_AND_BOBS_H

union tree_node;

namespace llvm { class Value; }

extern void llvm_set_decl (union tree_node *, Value *);
extern Value *llvm_get_decl(union tree_node *);
#define DECL_LLVM(NODE) (llvm_get_decl(NODE))
#define SET_DECL_LLVM(NODE, LLVM) (llvm_set_decl (NODE,LLVM))

/* Returns nonzero if the DECL_LLVM for NODE has already been set.  */
extern bool llvm_set_decl_p(union tree_node *);
#define DECL_LLVM_SET_P(NODE) (HAS_RTL_P (NODE) && llvm_set_decl_p(NODE))

/* The DECL_LLVM for NODE, if it is set, or NULL, if it is not set.  */
#define DECL_LLVM_IF_SET(NODE) \
  (DECL_LLVM_SET_P (NODE) ? DECL_LLVM (NODE) : NULL)

// emit_global_to_llvm - Emit the specified VAR_DECL to LLVM as a global
// variable.
// FIXME: Should not be here
void emit_global_to_llvm(union tree_node*);

extern bool flag_odr;

#endif
