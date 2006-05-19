//===-- hlvm/Base/URI.cpp - Uniform Resource Identifier ---------*- C++ -*-===//
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
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::Base::URI
//===----------------------------------------------------------------------===//

#include <hlvm/Base/URI.h>
#include <hlvm/Base/Memory.h>
#include <iostream>

namespace hlvm { namespace Base {

URI::URI( const char * text)
{
  apr_uri_parse(POOL, text, &uri_);
}

URI::URI( const std::string& str)
{
  apr_uri_parse(POOL, str.c_str(), &uri_);
}

URI::~URI ( void )
{
}

std::string 
URI::as_string( void ) const
{
  const char* result = apr_uri_unparse(POOL, &uri_, 0);
  return std::string( result );
}

/// @brief Clears the URI, releases memory.
void 
URI::clear( void )
{
  /// FIXME: how do we do this with the APR pools?
}

std::string 
URI::resolveToFile() const
{
  const char* scheme = this->scheme();
  if (scheme)
  {
    if (strncmp("file",this->scheme(),4) == 0)
    {
      return this->path();
    }
  }
  return "";
}

}}
