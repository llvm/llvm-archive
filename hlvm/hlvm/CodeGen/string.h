#include <stdint.h>

extern "C" {

typedef uint32_t _hlvm_size;
typedef char _hlvm_char;

struct _hlvm_string {
  _hlvm_size len;
  _hlvm_char* ptr;
};

extern void _hlvm_free_array(
  void* array, 
  _hlvm_size count, 
  _hlvm_size elem_size
);

extern void _hlvm_string_clear(_hlvm_string* str);
}
