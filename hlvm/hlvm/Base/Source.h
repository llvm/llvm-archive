//===-- AST Abstract Input Source Class -------------------------*- C++ -*-===//
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
/// @file hlvm/Base/Source.h
/// @author Reid Spencer <reid@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::Base::Source
//===----------------------------------------------------------------------===//

#ifndef HLVM_BASE_SOURCE_H
#define HLVM_BASE_SOURCE_H

#include <hlvm/Base/URI.h>
#include <llvm/System/MappedFile.h>
#include <istream>

namespace hlvm { namespace Base {
  /// This class is the base class of a family of input source classes that can
  /// be used to provide input to some parsing facility. This abstracts away
  /// the details of how the input is acquired. Four functions must be 
  /// implemented by subclasses: prepare, more, read, and finish.
  /// @brief Abstract Input Source Class
  class Source
  {
  /// @name Methods
  /// @{
  public:
    /// @brief This destructor does nothing, but declared virtual for subclasses
    virtual ~Source();

    /// @brief Tells the source to prepare to be read
    virtual void prepare(intptr_t block_len) = 0;

    /// @brief Requests a block of data.
    virtual const char* read(intptr_t& actual_len) = 0;

    /// @brief Indicates if more data waits
    virtual bool more() = 0;

    /// @brief Tells the source to finish up.
    virtual void finish() = 0;

    /// @brief Get the system identifier of the source
    virtual std::string systemId() const = 0;

    /// @brief Get the public identifier of the source
    virtual std::string publicId() const = 0;

  /// @}
  };

  Source* new_MappedFileSource(llvm::sys::MappedFile& mf);
  Source* new_StreamSource(std::istream&, std::string sysId = "<istream>", 
      size_t bSize = 65536);
  Source* new_URISource(const URI& uri);

}}

#endif
