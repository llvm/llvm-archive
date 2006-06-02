//===-- HLVM Memory Pool Implementation -------------------------*- C++ -*-===//
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
/// @file hlvm/Base/Pool.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/01
/// @since 0.1.0
/// @brief Defines the hlvm::Pool class 
//===----------------------------------------------------------------------===//

#include <hlvm/Base/Pool.h>
#include <hlvm/Base/Assert.h>
#include <llvm/ADT/StringExtras.h>

// Set up the maximum amount of APR pool debugging if we're in debug mode
// This must be set before including apr_pools.h
#ifdef HLVM_DEBUG
#define APR_POOL_DEBUG 0xFF
#endif
#include <apr-1/apr_pools.h>

namespace {

/// Base class for all pool implementations
class PoolBase : public hlvm::Pool {
  protected:
    PoolBase(
      const std::string& name, Pool* parent, const char * where 
    )
      : Pool(name,parent)
    {
      if (parent)
        allocator = 0;
      else
        if (APR_SUCCESS != apr_allocator_create(&allocator))
          hlvm::panic("Can't create APR allocator");

#ifdef HLVM_DEBUG
      if (APR_SUCCESS != apr_pool_create_ex_debug(&pool,
          (parent?static_cast<const PoolBase*>(parent->getParent())->pool:0), 
           aborter, allocator, where))
#else
      if (APR_SUCCESS != apr_pool_create_ex(&pool,
          (parent?static_cast<const PoolBase*>(parent->getParent())->pool:0), 
           aborter, allocator))
#endif
        hlvm::panic("Can't create APR pool");
    }
    virtual void* getAprPool() {
      return pool;
    }
  protected:
    static int aborter(int retcode) {
      std::string msg("Pool abort: retcode=");
      msg += llvm::itostr(retcode);
      hlvm::panic(msg.c_str());
      return 0;
    }

  protected:
    apr_pool_t* pool;
    apr_allocator_t* allocator;
};

/// A non-deallocating pool. This is fast, but should only be used for pools
/// where it doesn't matter if the objects in the pool can be deallocated. 
class ConstPool : public PoolBase {
  /// @name Constructors
  /// @{
  public:
    ConstPool(const std::string& name, Pool* parent, const char * where)
      : PoolBase(name,parent,where)
    {
    }
  /// @}
  /// @name Allocators
  /// @{
  public:
    virtual void* allocate(size_t block_size, const char* where) {
      return 0;
    }

    virtual void* callocate(size_t block_siz, const char* where ) {
      return 0;
    }

    virtual void deallocate(void* block, const char* where) {
    }

    virtual void clear(const char* where) {
    }
  /// @}
};

/// A very efficient pool for creating lots of small objects in a range of
/// sizes from tiny to moderate. This re-uses deallocated blocks of a similar
/// size so a new allocation is not needed. Handles memory churn of small 
/// objects well.
class SmallPool : public PoolBase {
  /// @name Constructors
  /// @{
  public:
    SmallPool(const std::string& name, Pool* parent, 
              uint32_t max_size, uint32_t increment, const char * where)
      : PoolBase(name,parent,where)
    {
    }
  /// @}
  /// @name Allocators
  /// @{
  public:
    virtual void* allocate(size_t block_size, const char* where) {
      return 0;
    }

    virtual void* callocate(size_t block_siz, const char* where ) {
      return 0;
    }

    virtual void deallocate(void* block, const char* where) {
    }

    virtual void clear(const char* where) {
    }
  /// @}
};

/// A simple allocator/deallocator similar to malloc. General purpose, nothing
/// fancy.
class SimplePool : public PoolBase {
  /// @name Constructors
  /// @{
  public:
    SimplePool(const std::string& name, Pool* parent, const char * where)
      : PoolBase(name,parent,where)
    {
    }
  /// @}
  /// @name Allocators
  /// @{
  public:
    virtual void* allocate(size_t block_size, const char* where) {
      return 0;
    }

    virtual void* callocate(size_t block_siz, const char* where ) {
      return 0;
    }

    virtual void deallocate(void* block, const char* where) {
    }

    virtual void clear(const char* where) {
    }
  /// @}
};

} // end anonymous namespace

namespace hlvm {

// Just to get the vtable in this file
Pool::~Pool()
{
}

// The interface to creating one of the pool classes
Pool*
Pool::create(
  const std::string& name,
  Pool* parent,
  bool no_dealloc,
  uint32_t max_elem_size,
  uint32_t increment,
  const char* where
)
{
  // If they don't want to deallocate, they always get a ConstPool
  if (no_dealloc)
    return new ConstPool(name,parent,where);

  // If they've specified an elem size and increment then if that combination
  // yields a table of less than 16K entries the we allocate a SmallPool. This
  // keeps the SmallPool's entry table under 64KBytes on a 32-bit machine.
  if (max_elem_size > 0 && increment > 0 && max_elem_size / increment <= 16384)
    return new SmallPool(name,parent,max_elem_size,increment,where);

  // Just use a standard pool
  return new SimplePool(name,parent,where);
}

void Pool::destroy(Pool* p, const char* where)
{
  p->clear(where);
  delete p;
}

} // end hlvm namespace
