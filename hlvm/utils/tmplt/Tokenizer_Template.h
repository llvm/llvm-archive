///////////////////////////////////////////////////////////////////////////////
/// @file hlvm/%MODULE_PATH%/%SCHEMA_NAME%Tokenizer.h
/// @author %AUTHOR%
/// @date %DATE%
/// @brief Declares the HLVM_%MODULE%::%SCHEMA_NAME%Tokenizer class.
///////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_%MODULE%_%SCHEMA_NAME%TOKENIZER_H
#define HLVM_%MODULE%_%SCHEMA_NAME%TOKENIZER_H

namespace HLVM_%MODULE%
{
  /// @brief The list of tokens for the $SCHEMA schema.
  enum %SCHEMA_NAME%Tokens
  {
    TKN_ERROR = -1,
    TKN_NONE  = 0,
    %TOKEN_LIST%
    TKN_COUNT
  };
  /// @brief Efficient token recognizer (perfect hash function) for the 
  /// %SCHEMA_NAME% schema
  class %SCHEMA_NAME%Tokenizer
  {
  /// @name Methods
  /// @{
  public:
    /// This function uses a fast perfect hash algorithm to convert the provided
    /// string into a numeric integer token. The set of strings supported are
    /// all the element, attribute and value names of the 
    /// %SCHEMA_NAME% Schema. 
    /// @param str The string to convert to a numeric token
    /// @return Returns an enumerated token value.
    /// @brief Convert a string token to an enumeration token, if possible.
    static int recognize( const char * str );

    /// @brief Lookup the name of a token by its value.
    static const char * lookup( int tkn );

  /// @}
  };
}

#endif
