#include "string.h"

extern "C" {

struct _hlvm_program_args {
  uint32_t argc;
  _hlvm_string* argv;
};

typedef uint32_t (*_hlvm_program_type)(_hlvm_program_args* args);

struct _hlvm_programs_element {
  const char* program_name;
  _hlvm_program_type program_entry;
};

extern uint32_t return0_prog(_hlvm_program_args* args);

_hlvm_programs_element _hlvm_programs[1] = {
  { "return0", return0_prog }
};

}
