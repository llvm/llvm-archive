// LLVM headers
#include "llvm/Constant.h"
#include "llvm/Value.h"

// GCC headers
#undef VISIBILITY_HIDDEN
#define IN_GCC

#include "config.h"
extern "C" {
#include "system.h"
}
#include "coretypes.h"
#include "target.h"
#include "tree.h"

using namespace llvm;

bool flag_odr = false;

void llvm_set_decl (tree t, Value *V) {
abort();
}

Value *llvm_get_decl(tree t) {
abort();
}

bool llvm_set_decl_p(tree t) {
abort();
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

extern "C" enum machine_mode ix86_getNaturalModeForType(tree type) {
abort();
}

extern "C" int ix86_HowToPassArgument(...) {
abort();
}

extern "C" int ix86_ClassifyArgument(...) {
abort();
}
