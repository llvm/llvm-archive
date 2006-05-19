///////////////////////////////////////////////////////////////////////////////
/// @author %AUTHOR%
/// @date %DATE%
/// @brief Implements the functions of class %SCHEMA_NAME%Tokenizer.
///////////////////////////////////////////////////////////////////////////////

#include <hlvm/%MODULE_PATH%/%SCHEMA_NAME%Tokenizer.h>
#include <hlvm/%MODULE_PATH%/%SCHEMA_NAME%TokenHash.i>

namespace HLVM_%MODULE% {

int
%SCHEMA_NAME%Tokenizer::recognize( const char * xml_str )
{
  const char* str = reinterpret_cast<const char*>( xml_str );
  const struct TokenMap *token_map = 
    %SCHEMA_NAME%TokenHash::in_word_set( str, strlen(str) );
  if (token_map)
  {
      return int(token_map->token);
  }
  return int(TKN_NONE);
}

const char *
%SCHEMA_NAME%Tokenizer::lookup( int tkn )
{
  for (unsigned int i = 0 ; i < sizeof(wordlist)/sizeof(wordlist[0]); i++)
  {
    if (tkn == wordlist[i].token) return wordlist[i].name;
  }
  return "";
}

}
