#include <stdint.h>

extern "C" {

typedef uint32_t _hlvm_size;
typedef uint16_t _hlvm_char;

struct _hlvm_string {
  _hlvm_size len;
  _hlvm_char* ptr;
};

extern void _hlvm_free_array(
  void* array, 
  _hlvm_size count, 
  _hlvm_size elem_size
);

void _hlvm_string_clear(_hlvm_string* str) 
{
  _hlvm_free_array(str->ptr,str->len,sizeof(_hlvm_char));
  str->len = 0;
  str->ptr = 0;
}

}
