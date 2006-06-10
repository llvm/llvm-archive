//===-- Runtime Text String Implementation ----------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Text.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/05/25
/// @since 0.1.0
/// @brief Implements the functions for runtime text string support
//===----------------------------------------------------------------------===//

#include <apr-1/apr_strings.h>

#include <hlvm/Runtime/Internal.h>

extern "C" {
#include <hlvm/Runtime/Text.h>
}

namespace {

class Text : public hlvm_text_obj {
};

}

extern "C" {

hlvm_text 
hlvm_text_create(const char* initializer)
{
  Text* result = new Text();
  if (initializer) {
    result->len = strlen(initializer);
    result->str = const_cast<char*>(initializer);
    result->is_const = true;
  } else {
    result->len = 0;
    result->str = 0;
    result->is_const = false;
  }
  return hlvm_text(result);
}

void
hlvm_text_delete(hlvm_text T)
{
  Text* t = reinterpret_cast<Text*>(T);
  if (!t->is_const && t->str)
    delete [] t->str;
  delete t;
}

}
