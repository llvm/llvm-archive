//===-- hlvm/Reader/Reader.h - AST Abstract Reader Class --------*- C++ -*-===//
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
/// @file hlvm/Reader/Reader.h
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::Reader
//===----------------------------------------------------------------------===//

#ifndef XPS_READER_READER_H
#define XPS_READER_READER_H

namespace hlvm {

class AST;

class Reader
{
public:
  virtual ~Reader() {}

  /// This method reads the entire content of the reader's source.
  virtual void read() = 0;

  /// This method retrieves the construct AST that resulted from reading.
  /// @returns 0 if nothing has been read yet
  virtual AST* get() = 0;
};

}
#endif
