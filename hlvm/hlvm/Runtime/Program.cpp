//===-- Runtime Program Implementation --------------------------*- C++ -*-===//
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
/// @file hlvm/Runtime/Program.cpp
/// @author Reid Spencer <rspencer@reidspencer.org> (original author)
/// @date 2006/06/04
/// @since 0.1.0
/// @brief Implements the runtime program facilities.
//===----------------------------------------------------------------------===//

#include <hlvm/Runtime/Program.h>
#include <string.h>

namespace {


}

extern "C" {

/// Declare the external function that gets use the first element in
/// the hlvm_programs appending array. This function is implemented in the
/// hlvm_get_programs.ll file.
extern hlvm_programs_element* hlvm_get_programs();

/// Search the list of programs for a matching entry point. This uses a dumb
/// linear search but it should be okay because the number of program entry
/// points in any given executable should not be huge. If it is, someone is
/// designing their software poorly and they deserve what they get :)
hlvm_program_type 
hlvm_find_program(const char* uri)
{
  hlvm_programs_element* p = hlvm_get_programs();
  while (p && p->program_entry) {
    if (strcmp(p->program_name,uri) == 0)
      return p->program_entry;
    ++p;
  }
  return 0;
}

}
