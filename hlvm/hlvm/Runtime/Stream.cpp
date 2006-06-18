//===-- Runtime Stream I/O Implementation -----------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Stream.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/24
/// @since 0.1.0
/// @brief Implements the functions for runtime stream input/output.
//===----------------------------------------------------------------------===//

#include <hlvm/Runtime/Internal.h>

extern "C" {
#include <hlvm/Runtime/Stream.h>
}

namespace 
{
  class Stream : public hlvm_stream_obj 
  {
  public:
    Stream* open(const char* uri)
    {
      if (uri[0] == 'h' && uri[1] == 'l' && uri[2] == 'v' && uri[3] == 'm') {
        if (0 == apr_strnatcmp(uri,"hlvm:std:out")) {
          if (HLVM_APR(apr_file_open_stdout(&this->fp,_hlvm_pool),"opening std:out"))
            return this;
        } else if (0 == apr_strnatcmp(uri,"hlvm::std::in")) {
          if (HLVM_APR(apr_file_open_stdin(&this->fp,_hlvm_pool),"opening std:in"))
            return this;
        } else if (0 == apr_strnatcmp(uri,"hlvm::std::err")) {
          if (HLVM_APR(apr_file_open_stderr(&this->fp,_hlvm_pool),"opening std:err"))
            return this;
        }
      }
      return 0;
    }
    void close() {
      HLVM_APR(apr_file_close(this->fp),"closing a stream");
    }
    hlvm_size write(void* data, hlvm_size len) {
      apr_size_t nbytes = len;
      HLVM_APR(apr_file_write(this->fp, data, &nbytes),"writing a stream");
      return nbytes;
    }
    hlvm_size write(const char* string) {
      apr_size_t nbytes = strlen(string);
      HLVM_APR(apr_file_write(this->fp, string, &nbytes),"writing a stream");
      return nbytes;
    }
  };
}

extern "C" {

hlvm_stream
hlvm_stream_open(const char* uri)
{
  hlvm_assert(uri);
  Stream* result = new Stream();
  return result->open(uri);
}

void 
hlvm_stream_close(hlvm_stream stream)
{
  hlvm_assert(stream);
  Stream* strm = static_cast<Stream*>(stream);
  strm->close();
}

hlvm_size
hlvm_stream_write_buffer(hlvm_stream stream, hlvm_buffer data, hlvm_size len)
{
  hlvm_assert(stream);
  hlvm_assert(data && data->data);
  Stream* strm = static_cast<Stream*>(stream);
  return strm->write(data->data,data->len);
}

hlvm_size
hlvm_stream_write_text(hlvm_stream stream, hlvm_text txt)
{
  hlvm_assert(stream);
  hlvm_assert(txt && txt->str);
  Stream* strm = static_cast<Stream*>(stream);
  return strm->write(txt->str,txt->len);
}

hlvm_size 
hlvm_stream_write_string(hlvm_stream stream, const char* string)
{
  hlvm_assert(stream);
  hlvm_assert(string);
  Stream* strm = static_cast<Stream*>(stream);
  return strm->write(string);
}

}
