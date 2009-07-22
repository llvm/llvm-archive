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

void llvm_set_decl (tree t, Value *V) {
  assert(HAS_RTL_P(t) && "Expected a gcc decl with RTL!");
  llvm_set_cached(t, V);
}

Value *llvm_get_decl(tree t) {
  assert(HAS_RTL_P(t) && "Expected a gcc decl with RTL!");
  return (Value *)llvm_get_cached(t);
}

bool llvm_set_decl_p(tree t) {
  assert(HAS_RTL_P(t) && "Expected a gcc decl with RTL!");
  llvm_has_cached(t);
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
