// LLVM headers
#include "llvm/Constant.h"
#include "llvm/Value.h"

// System headers
#include <gmp.h>

// GCC headers
#undef VISIBILITY_HIDDEN

extern "C" {
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
}

// Plugin headers
extern "C" {
#include "llvm-cache.h"
}

using namespace llvm;

bool flag_odr = false;

Value *llvm_set_decl (tree t, Value *V) {
  assert(HAS_RTL_P(t) && "Expected a gcc decl with RTL!");
  llvm_set_cached(t, V);
  return V;
}

extern Value *make_decl_llvm(tree decl);

Value *llvm_get_decl(tree t) {
  assert(HAS_RTL_P(t) && "Expected a gcc decl with RTL!");
  if (Value *V = (Value *)llvm_get_cached(t))
    return V;
  return make_decl_llvm(t);
}

bool llvm_set_decl_p(tree t) {
  assert(HAS_RTL_P(t) && "Expected a gcc decl with RTL!");
  return llvm_has_cached(t);
}

void eraseLocalLLVMValues() {
abort();
}

void changeLLVMConstant(Constant *Old, Constant *New) {
abort();
}

void llvmEraseLType(const Type *Ty) {
abort();
}

int ix86_regparm;

extern "C" bool contains_aligned_value_p (tree type) {
abort();
}
