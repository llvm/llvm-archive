#include "string.cxx"

extern "C" {

struct _hlvm_program_args {
  uint32_t argc;
  _hlvm_string* argv;
};

typedef uint32_t (*_hlvm_program_type)(_hlvm_program_args* args);

struct _hlvm_programs_element {
  void* program_type;
  _hlvm_string program_name;
  _hlvm_program_type program_entry;
};

_hlvm_programs_element _hlvm_programs[1];

}
