//===-- hlvm/Base/Memory.h - HLVM Memory Facilities -------------*- C++ -*-===//
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
/// @file hlvm/Base/Memory.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares memory facilities for HLVM
//===----------------------------------------------------------------------===//

#ifndef HLVM_BASE_MEMORY_H
#define HLVM_BASE_MEMORY_H

#include <apr-1/apr_pools.h>

/// The BASE module implements fundamental utilities at the core of XPS.
/// @brief The most elemental module in XPS.
namespace hlvm { namespace Base {

/// An APR pool to allocate memory into. This is the master pool of all XPS
/// allocated pools and generally the one used to allocate APRish stuff into.
/// Note that this doesn't exist until after the call to initialize()
/// @see initialize
/// @brief The main memory pool for XPS
extern apr_pool_t* POOL;

/// Determine if emergency memory reserve has been exhausted.
/// @brief Detemine if heap memory is low.
extern bool memory_is_low( void );

void initialize(int& argc, char**argv );
void terminate();

}}

void* operator new(size_t size, apr_pool_t* pool);
void* operator new[](size_t size, apr_pool_t* pool);

#endif
