//===-- hlvm/Base/Source.cpp - AST Abstract Data Sourc Class ----*- C++ -*-===//
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
/// @author Reid Spencer <reid@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Defines the class hlvm::Source
//===----------------------------------------------------------------------===//

#include <hlvm/Base/Source.h>
#include <llvm/System/MappedFile.h>
#include <iostream>
#include <ios>

using namespace hlvm;

namespace 
{

class MappedFileSource : public Base::Source 
{
public:
  MappedFileSource(llvm::sys::MappedFile& mf)
    : mf_(mf), at_(0), end_(0), was_mapped_(false), read_len_(0)
  {
  }

  virtual void prepare(intptr_t block_len) 
  {
    was_mapped_ = mf_.isMapped();
    if (!was_mapped_)
      mf_.map();
    at_ = mf_.charBase();
    end_ = at_ + mf_.size();
    read_len_ = block_len;
  }

  virtual bool more()
  {
    return at_ < end_;
  }

  virtual const char* read(intptr_t& actual_len)
  {
    if (at_ >= end_) {
      actual_len = 0;
      return 0;
    }
    const char* result = at_;
    if (read_len_ < end_ - at_) {
      actual_len = read_len_;
      at_ += read_len_;
    } else {
      actual_len = end_ - at_;
      at_ = end_;
    }
    return result;
  }

  virtual void finish()
  {
    if (!was_mapped_)
      mf_.unmap();
    at_ = 0;
    end_ = 0;
    was_mapped_ = false;
    read_len_ = 0;
  }

  virtual std::string systemId() const
  {
    return mf_.path().toString();
  }

  virtual std::string publicId() const
  {
    return "file://" + systemId();
  }

private:
  llvm::sys::MappedFile& mf_;
  const char* at_;
  const char* end_;
  bool was_mapped_;
  intptr_t read_len_;
};

class StreamSource : public Base::Source 
{
public:
  StreamSource(std::istream& strm, std::string sysId, size_t bsize) : s_(strm) 
  {
    s_.seekg(0, std::ios::end);
    len_ = s_.tellg();
    s_.seekg(0, std::ios::beg);
    if (bsize > 1024*1024)
      buffSize = 1024*1024;
    else 
      buffSize = bsize;
    buffer = new char[buffSize+1];
  }

  virtual ~StreamSource()
  {
    delete buffer;
  }

  virtual void prepare(intptr_t block_len)
  {
    s_.seekg(0,std::ios::beg);
  }

  virtual bool more()
  {
    return ! s_.bad() && !s_.eof();
  }

  virtual const char* read(intptr_t& actual_len)
  {
    if (more())
    {
      size_t pos = s_.tellg();
      if (len_ - pos > buffSize)
        actual_len = buffSize;
      else
        actual_len = len_ - pos;
      s_.read(buffer, actual_len);
      buffer[actual_len] = 0;
      return buffer;
    }
    else
    {
      buffer[0] = 0;
      actual_len = 0;
      return buffer;
    }
  }

  virtual void finish()
  {
  }

  virtual std::string systemId() const
  {
    return sysId;
  }

  virtual std::string publicId() const
  {
    return "file:///" + systemId();
  }

private:
  std::string sysId;
  std::istream& s_;
  size_t len_;
  char* buffer;
  size_t buffSize;
};

class URISource : public Base::Source 
{
private:
  hlvm::Base::URI uri_;
  llvm::sys::MappedFile* mf_;
  MappedFileSource* mfs_;
public:
  URISource(const Base::URI& uri ) 
    : uri_(uri) 
    , mf_(0)
    , mfs_(0)
  {
    mf_ = new llvm::sys::MappedFile(llvm::sys::Path(uri_.resolveToFile()));
    mfs_ = new MappedFileSource(*mf_);
  }
  virtual ~URISource() {}
  virtual void prepare(intptr_t block_len) { mfs_->prepare(block_len); }
  virtual bool more() { return mfs_->more(); }
  virtual const char* read(intptr_t& actual_len) 
  { return mfs_->read(actual_len); }
  virtual void finish() { mfs_->finish(); }
  virtual std::string systemId() const { return mfs_->systemId(); }
  virtual std::string publicId() const { return "file://" + mfs_->systemId(); }
};

}

namespace hlvm { namespace Base {

Source::~Source() 
{
}

Source* 
new_MappedFileSource(llvm::sys::MappedFile& mf)
{
  return new MappedFileSource(mf);
}


Source* 
new_StreamSource(std::istream& strm, std::string sysId, size_t bSize)
{
  return new StreamSource(strm, sysId, bSize);
}

Source* 
new_URISource(const Base::URI& uri)
{
  return new URISource(uri);
}

}}
