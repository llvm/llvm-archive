#include "string.h"

extern "C" {

void _hlvm_string_clear(_hlvm_string* str) 
{
  _hlvm_free_array(str->ptr,str->len,sizeof(_hlvm_char));
  str->len = 0;
  str->ptr = 0;
}

}
