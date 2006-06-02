//===-- HLVM Memory Pool Facilities -----------------------------*- C++ -*-===//
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
/// @file hlvm/Base/Pool.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/01
/// @since 0.1.0
/// @brief Declares the hlvm::Pool class and memory pooling facilities for HLVM
//===----------------------------------------------------------------------===//

#ifndef HLVM_BASE_POOL_H
#define HLVM_BASE_POOL_H

#ifdef HLVM_DEBUG
#define HLVM_STRINGIZE(X) #X
#define HLVM_NEW(pool,type,args) \
  (::new(pool,__FILE__ ":" HLVM_STRINGIZE(__LINE__)) type args )
#else
#define HLVM_NEW(pool,type,args) \
  (::new(pool) type args )
#endif

#include <string>

namespace hlvm {

/// A memory pool abstraction. This is the is is the master pool of all XPS
/// allocated pools and generally the one used to allocate APRish stuff into.
/// Note that this doesn't exist until after the call to initialize()
/// @brief The memory pool for HLVM.
class Pool {

  /// @name Constructors
  /// @{
  protected:
    Pool(const std::string& name, Pool* parent) {}
    virtual ~Pool();
  public:
    static Pool* create(
      const std::string& name = "", ///< name for the pool
      Pool* parent = 0,             ///< parent pool, 0 = root pool
      bool no_dealloc = false,      ///< allow deallocations?
      uint32_t max_elem_size = 0,   ///< maximum allocation size, 0=max
      uint32_t increment = 8,       ///< min difference between alloc sizes
      const char * where = 0        ///< location of the pool creation
    );

    static void destroy(Pool* p, const char* where = 0);

  /// @}
  /// @name Allocators
  /// @{
  public:
    /// Allocate a block of data of size block_size and return it
    virtual void* allocate(size_t block_size, const char* where = 0) = 0;

    /// Allocate a block of data of size block_size, zero its content  and 
    /// return it
    virtual void* callocate(size_t block_siz, const char* where = 0 ) = 0;

    /// Deallocate a block of data
    virtual void deallocate(void* block, const char* where = 0) = 0;

    /// Clear all memory in the pool and any sub-pools
    virtual void clear(const char* where = 0) = 0; 

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getName() { return name; }
    const Pool* getParent() { return parent; }
    virtual void* getAprPool() = 0;

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;
    Pool* parent;
  /// @}
};

} // end hlvm namespace

#ifdef HLVM_DEBUG
inline void* operator new(size_t size, hlvm::Pool* p, const char* where)
{
  return p->callocate(size,where);
}

inline void* operator new[](size_t size, hlvm::Pool* p, const char* where)
{
  return p->callocate(size,where);
}
#else
inline void* operator new(size_t size, hlvm::Pool* p)
{
  return p->callocate(size);
}

inline void* operator new[](size_t size, hlvm::Pool* p)
{
  return p->callocate(size);
}
#endif

#endif
