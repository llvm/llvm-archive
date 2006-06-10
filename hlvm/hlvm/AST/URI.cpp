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
/// @file hlvm/AST/URI.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::Base::URI
//===----------------------------------------------------------------------===//

#include <hlvm/AST/URI.h>
#include <hlvm/Base/Pool.h>
#include <apr-1/apr_uri.h>
#include <apr-1/apr_pools.h>
#include <iostream>

namespace 
{

class URIImpl : public hlvm::URI 
{
  apr_pool_t* pool;
  apr_uri_t uri_;

public:
  URIImpl(hlvm::Pool* p) 
    : pool(reinterpret_cast<apr_pool_t*>(p->getAprPool())) {}

  virtual const char* const scheme() const { return uri_.scheme; }
  virtual const char* const password() const { return uri_.password; }
  virtual const char* const user() const { return uri_.user; }
  virtual const char* const hostname() const { return uri_.hostname; }
  virtual uint32_t port() const { return atoi(uri_.port_str); }
  virtual const char* const path() const { return uri_.path; }
  virtual const char* const query() const { return uri_.query; }
  virtual const char* const fragment() const { return uri_.fragment; }

  virtual hlvm::URI& assign( const hlvm::URI& that ) {
    this->uri_ = reinterpret_cast<const URIImpl&>(that).uri_;
    return *this;
  }

  virtual const char* const hostinfo() const { return ""; }
  virtual bool equals(const hlvm::URI& uri) const { 
    if (&uri == this)
      return true;
    return uri.as_string() == this->as_string();
  }

  virtual void assign( const std::string& that ) {
    apr_uri_parse(pool, that.c_str(), &uri_);
  }
  virtual void clear( void ) {}
  virtual void scheme( const char* const scheme ) {}
  virtual void opaque( const char* const opaque ) {}
  virtual void authority( const char* const authority ) {}
  virtual void user( const char* const user ) {}
  virtual void server( const char* const server ) {}
  virtual void port( uint32_t portnum ) {}

  virtual std::string as_string( void ) const {
    const char* result = apr_uri_unparse(pool, &uri_, 0);
    return std::string( result );
  }

  void get_string(std::string& str) const {
    str = apr_uri_unparse(pool, &uri_, 0);
  }

  std::string resolveToFile() const {
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
};

}

namespace hlvm {

URI::~URI ( void )
{
}

URI*
URI::create( const std::string& str, Pool* p)
{
  URIImpl* result = new URIImpl(p);
  result->assign(str);
  return result;
}

}
