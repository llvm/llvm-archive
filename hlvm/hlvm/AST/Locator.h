//===-- hlvm/AST/Locator.h - AST Location Class -----------------*- C++ -*-===//
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
/// @file hlvm/AST/Locator.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Locator
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_LOCATOR_H
#define HLVM_AST_LOCATOR_H

#include <string>

namespace hlvm {
namespace AST {

  /// This class is used to hold a source code location as a filename, line
  /// number and column number. This is used for generating error messages and
  /// for debugging support.
  /// @brief Source location holder class.
  class Locator
  {
    /// @name Constructors
    /// @{
    public:
      Locator(uint32_t line, uint32_t col, const std::string* fname)
        : line_(line), col_(col), fname_(fname) {}
      Locator() : line_(0), col_(0), fname_(0) {}

    /// @}
    /// @name Accessors
    /// @{
    public:

    /// @}
    /// @name Data
    /// @{
    protected:
      uint32_t line_;           ///< Line number of source location
      uint32_t col_;            ///< Column number of source location
      const std::string* fname_;///< File name of source location
    /// @}
  };
} // AST
} // hlvm
#endif
