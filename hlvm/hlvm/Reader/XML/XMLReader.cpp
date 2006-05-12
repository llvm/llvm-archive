//===-- hlvm/Reader/XML/XMLReader.cpp - AST XML Reader Class ----*- C++ -*-===//
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
/// @file hlvm/Reader/XML/XMLReader.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::XMLReader
//===----------------------------------------------------------------------===//

#include <hlvm/Reader/XML/XMLReader.h>
#include <hlvm/AST/AST.h>

using namespace hlvm;

namespace {

class XMLReaderImpl : public XMLReader {
public:
  XMLReaderImpl(const llvm::sys::Path& path) :
    path_(path), ast_(0) {}

  virtual ~XMLReaderImpl() { if (ast_) delete ast_; }

  virtual void read();
  virtual AST::AST* get();

private: 
  llvm::sys::Path path_;
  AST::AST* ast_;
};

AST::AST*
XMLReaderImpl::get()
{
  return ast_;
}

void
XMLReaderImpl::read() {
  ast_ = new AST::AST();
}

}

XMLReader* 
XMLReader::create(const llvm::sys::Path& path)
{
  return new XMLReaderImpl(path);
}
