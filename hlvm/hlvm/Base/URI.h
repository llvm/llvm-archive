//===-- Uniform Resource Identifier -----------------------------*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/Base/URI.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::Base::URI
//===----------------------------------------------------------------------===//

#ifndef HLVM_BASE_URI_H
#define HLVM_BASE_URI_H

#include <apr-1/apr_uri.h>
#include <string>

namespace hlvm { namespace Base {

/// A class to represent Uniform Resource Identifiers (URIs). This class can
/// also support URLs and URNs. The implementation is based on the APR-UTIL
/// apr_uri set of functions and associated data types. 
/// @see RFC 2396 states that hostnames take the form described in 
/// @see RFC 1034 (Section 3) Hostname Syntax
/// @see RFC 1123 (Section 2.1). Hostnames
/// @see RFC 1609 Universal Resource Identifiers in WWW - Berners-Lee.
/// @see RFC 1738 Uniform Resource Locators - Berners-Lee.
/// @see RFC 1808 Relative Uniform Resource Locators - Fielding
/// @see RFC 2059 Uniform Resource Locators for z39.50 - Denenberg
/// @see RFC 2111 Content-ID and Message-ID Uniform Resource Locators-Levinson
/// @see RFC 2396 Uniform Resource Identifiers (URI) - Berners-Lee
/// @see RFC 3305 URI/URL/URN Clarifications and Recommendations - Mealling
/// @see RFC 3406 URN Namespace Definition Mechanisms - Daigle
/// @see RFC 3508 URL Schem Registration - Levin
/// @brief HLVM Uniform Resource Identifier Class
class URI
{
/// @name Constructors
/// @{
public:
  /// @brief Default Constructor builds empty uri
  URI ( void );

  /// @brief Parsing constructor builds uri by parsing the string parameter
  URI( const char * str );

  /// @brief Parsing constructor builds uri by parsing the string parameter
  URI( const std::string& xmlStr);

  /// @brief Copy Constructor.
  URI ( const URI & that );

  /// @brief Destructor.
  ~URI ( void );

/// @}
/// @name Operators
/// @{
public:
  /// @brief Equality Operator.
  bool operator == ( const URI & that ) const;

  /// @brief Assignment Operator.
  URI & operator = ( const URI & that );

/// @}
/// @name Accessors
/// @{
public:

  /// @brief Return the URI as a string (escaped). Caller must
  /// free the returned string.
  std::string as_string( void ) const;

  /// @brief Set a string to the value of the URI with escapes.
  void get_string( std::string& str_to_fill ) const;

  /// @brief Return the scheme of the URI
  const char* const scheme() const;

  /// @brief Return the password part of the URI
  const char* const password() const;

  /// @brief Return the user part of the URI
  const char* const user() const;

  /// @brief Return the host name from the URI
  const char* const hostname() const;

  /// @brief Return the combined [user[:password]\@host:port/ part of the URI
  const char* const hostinfo() const;

  /// @brief Return the port part of the URI
  uint32_t port() const;

  /// @brief Return the path part of the URI
  const char* const path() const;

  /// @brief Return the query part of the URI
  const char* const query() const;

  /// @brief Return the fragment identifier part of the URI
  const char* const fragment() const;

  /// @brief Determines if two URIs are equal
  bool equals( const URI& that ) const;

  std::string resolveToFile() const;
/// @}
/// @name Mutators
/// @{
public:
  /// @brief Assignment of one URI to another
  URI& assign( const URI& that );

  /// @brief Assignment of an std::string to a URI. 
  void assign( const std::string& that );

  /// @brief Clears the URI, releases memory.
  void clear( void );

  /// @brief Sets the scheme
  //void scheme( const char* const scheme );

  /// @brief Set the opaque part of the URI
  //void opaque( const char* const opaque );

  /// @brief Sets the authority.
  //void authority( const char* const authority );

  /// @brief Sets the user.
  //void user( const char* const user );

  /// @brief Sets the server.
  //void server( const char* const server );

  /// @brief Sets the port.
  //void port( uint32_t portnum );

/// @}
/// @name Functions
/// @{
public:
  static uint32_t port_for_scheme( const char* scheme );

/// @}
/// @name Data
/// @{
private:
  apr_uri_t uri_;
/// @}
/// @name Unimplemented
/// @{
private:
/// @}

};

inline
URI::URI()
{
}

inline URI&
URI::assign( const URI& that )
{
  this->uri_ = that.uri_;
  return *this;
}

inline
URI::URI ( const URI & that )
{
  this->assign(that);
}

inline URI & 
URI::operator = ( const URI & that )
{
  return this->assign(that);
}

inline bool 
URI::operator == ( const URI & that ) const
{
  return this->equals(that);
}

inline const char* const
URI::scheme() const
{
  return uri_.scheme;
}

inline const char* const
URI::password() const
{
  return uri_.password;
}

inline const char* const
URI::user() const 
{
  return uri_.user;
}

inline const char* const
URI::hostname() const 
{
  return uri_.hostname;
}

inline uint32_t 
URI::port() const 
{
  return atoi(uri_.port_str);
}

inline const char* const
URI::path() const 
{
  return uri_.path;
}

inline const char* const
URI::query() const 
{
  return uri_.query;
}

inline const char* const
URI::fragment() const 
{
  return uri_.fragment;
}

}}

#endif
