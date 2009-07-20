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

const Type *llvm_set_type(tree Tr, const Type *Ty) {
abort();
}

const Type *llvm_get_type(unsigned Index) {
abort();
}
