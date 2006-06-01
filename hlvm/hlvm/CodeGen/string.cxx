#include <stdint.h>

extern "C" {

typedef uint32_t _hlvm_size;
typedef uint16_t _hlvm_char;

struct _hlvm_string {
  _hlvm_size len;
  _hlvm_char* ptr;
};

extern void _hlvm_string_clear(_hlvm_string* str); 

_hlvm_string astring;

}
