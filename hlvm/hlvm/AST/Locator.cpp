//===-- AST Locator Classes Implementation ----------------------*- C++ -*-===//
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
/// @file hlvm/AST/Locator.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Defines the methods of class hlvm::AST::Locator and friends
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Locator.h>
#include <hlvm/Base/Assert.h>
#include <llvm/ADT/StringExtras.h>

namespace hlvm
{

Locator::~Locator() {}
URILocator::~URILocator() {}
LineLocator::~LineLocator() {}
LineColumnLocator::~LineColumnLocator() {}
RangeLocator::~RangeLocator() {}

bool 
URILocator::equals(const Locator& that) const
{
  if (this == &that)
    return true;
  if (that.id() >= this->SubclassID)
    return this->uri == static_cast<const URILocator&>(that).uri;
  return false;
}

void 
URILocator::getReference(std::string& ref) const
{
  hlvmAssert(uri != 0);
  ref = uri->as_string();
}

void 
LineLocator::getReference(std::string& ref) const
{
  URILocator::getReference(ref);
  ref += ":" + llvm::utostr(line);
}

bool 
LineLocator::equals(const Locator& that) const
{
  if (this == &that)
    return true;
  if (that.id() >= this->SubclassID)
    return URILocator::equals(that) && 
      this->line == static_cast<const LineLocator&>(that).line;
  return false;
}
void
LineColumnLocator::getReference(std::string& ref) const
{
  LineLocator::getReference(ref);
  ref += ":" + llvm::utostr(col);
}

bool
LineColumnLocator::equals(const Locator& that) const
{
  if (this == &that)
    return true;
  if (that.id() >= this->SubclassID)
    return LineLocator::equals(that) && 
      this->col == static_cast<const LineColumnLocator&>(that).line;
  return false;
}

void
RangeLocator::getReference(std::string& ref) const
{
  URILocator::getReference(ref);
  ref += "(" + llvm::utostr(line) + ":" + llvm::utostr(col) + "," 
             + llvm::utostr(line2) + ":" + llvm::utostr(col2) + ")"; 
}

bool
RangeLocator::equals(const Locator& that) const
{
  if (this == &that)
    return true;
  if (that.id() >= this->SubclassID)
    return LineColumnLocator::equals(that) && 
      this->line2 == static_cast<const RangeLocator&>(that).line2 &&
      this->col2 == static_cast<const RangeLocator&>(that).col2 ;
  return false;
}
} // hlvm
