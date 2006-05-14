//===-- hlvm/Writer/XML/XMLWriter.cpp - AST XML Writer Class ----*- C++ -*-===//
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
/// @file hlvm/Writer/XML/XMLWriter.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::XMLWriter
//===----------------------------------------------------------------------===//

#include <hlvm/Writer/XML/XMLWriter.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <iostream>
#include <fstream>

using namespace hlvm;

namespace {

class XMLWriterImpl : public XMLWriter {
  llvm::sys::Path path_;
  std::fstream* out_;
  AST::AST* ast_;
public:
  XMLWriterImpl(const llvm::sys::Path& path) :
    path_(path), out_(0), ast_(0)
  {
  }

  virtual ~XMLWriterImpl() 
  { 
    if (out_) {
      out_->flush();
      out_->close();
      delete out_;
    }
  }

  virtual void write(AST::AST* ast);
};

void
XMLWriterImpl::write(AST::AST* ast) {
  ast_ = ast;
}

}

XMLWriter* 
XMLWriter::create(const llvm::sys::Path& path)
{
  return new XMLWriterImpl(path);
}
