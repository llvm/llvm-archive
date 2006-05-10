//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Location.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Location
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_LOCATION_H
#define HLVM_AST_LOCATION_H

#include <string>

namespace hlvm {
namespace AST {

  /// This class is used to hold a source code location as a filename, line
  /// number and column number. This is used for generating error messages and
  /// for debugging support.
  /// @brief Source location holder class.
  class Location
  {
    /// @name Constructors
    /// @{
    public:
      Location(uint32_t line, uint32_t col, const std::string& fname)
        : line_(line), col_(col), fname_(fname)  {}

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
      std::string fname_;       ///< File name of source location
    /// @}
  };
} // AST
} // hlvm
#endif
