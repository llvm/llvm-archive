//===-- Uniform Resource Identifier Interface -------------------*- C++ -*-===//
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
/// @file hlvm/AST/URI.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::Base::URI
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_URI_H
#define HLVM_AST_URI_H

#include <string>
#include <stdint.h>

namespace hlvm {

// Forward References
class AST; 
class Pool;

/// A class to represent Uniform Resource Identifiers (URIs). This class can
/// also support URLs and URNs. HLVM uses URIs to uniquely identify bundles and
/// as the operand of OpenOp operators to specify the resource to be opened.
/// HLVM defines its own namespace, "hlvm", in which certain classes of 
/// resources can be specified.
/// @see RFC 2396 Uniform Resource Identifiers (URI): Generic Syntax
/// @see RFC 1034 HEMS Variable Definitions (Section 3, Hostname Syntax)
/// @see RFC 1123 Requirements for Internet Hosts (Section 2.1, Hostnames)
/// @see RFC 1738 Uniform Resource Locators (URL)
/// @see RFC 1808 Relative Uniform Resource Locators
/// @see RFC 2111 Content-ID and Message-ID Uniform Resource Locators
/// @see RFC 3305 URI/URL/URN Clarifications and Recommendations
/// @see RFC 3406 URN Namespace Definition Mechanisms
/// @see RFC 3508 H.323 URL Scheme Registration 
/// @brief HLVM Uniform Resource Identifier Class
class URI
{
  /// @name Constructors
  /// @{
  protected:
    // This is an abstract class, don't allow construction or copying
    URI () {}
    URI ( const URI & that ) {}
    /// @brief Parsing constructor builds uri by parsing the string parameter
    static URI* create( const std::string& uriStr, Pool* p);
    virtual ~URI ( void );

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
    virtual std::string as_string( void ) const = 0;

    /// @brief Set a string to the value of the URI with escapes.
    virtual void get_string( std::string& str_to_fill ) const = 0;

    /// @brief Return the scheme of the URI
    virtual const char* const scheme() const = 0;

    /// @brief Return the password part of the URI
    virtual const char* const password() const = 0;

    /// @brief Return the user part of the URI
    virtual const char* const user() const = 0;

    /// @brief Return the host name from the URI
    virtual const char* const hostname() const = 0;

    /// @brief Return the combined [user[:password]\@host:port/ part of the URI
    virtual const char* const hostinfo() const = 0;

    /// @brief Return the port part of the URI
    virtual uint32_t port() const = 0;

    /// @brief Return the path part of the URI
    virtual const char* const path() const = 0;

    /// @brief Return the query part of the URI
    virtual const char* const query() const = 0;

    /// @brief Return the fragment identifier part of the URI
    virtual const char* const fragment() const = 0;

    /// @brief Determines if two URIs are equal
    virtual bool equals( const URI& that ) const = 0;

    virtual std::string resolveToFile() const = 0;
  /// @}
  /// @name Mutators
  /// @{
  public:
    /// @brief Assignment of one URI to another
    virtual URI& assign( const URI& that ) = 0;

    /// @brief Assignment of an std::string to a URI. 
    virtual void assign( const std::string& that ) = 0;

    /// @brief Clears the URI, releases memory.
    virtual void clear( void ) = 0;

    /// @brief Sets the scheme
    virtual void scheme( const char* const scheme ) = 0;

    /// @brief Set the opaque part of the URI
    virtual void opaque( const char* const opaque ) = 0;

    /// @brief Sets the authority.
    virtual void authority( const char* const authority ) = 0;

    /// @brief Sets the user.
    virtual void user( const char* const user ) = 0;

    /// @brief Sets the server.
    virtual void server( const char* const server ) = 0;

    /// @brief Sets the port.
    virtual void port( uint32_t portnum ) = 0;

  /// @}
  /// @name Functions
  /// @{
  public:
    static uint32_t port_for_scheme( const char* scheme );
  /// @}

  friend class AST;
};

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

} // end hlvm namespace

#endif
