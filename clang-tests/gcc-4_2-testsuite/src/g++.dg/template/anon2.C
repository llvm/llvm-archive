// PR c++/28279
// finish_static_data_member_decl was confused by the anonymous
// namespace causing TREE_PUBLIC to be unset

template<typename T>
struct is_pointer_impl {
    static const bool value = true; /* { dg-error "has internal linkage but is not defined" } */
};

namespace {
  class prefix_name_mapper {};
}

static const bool val = is_pointer_impl<prefix_name_mapper>::value; /* { dg-error "note: used here" } */

